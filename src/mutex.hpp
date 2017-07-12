#pragma once

/**
 * This is the header needed to use "Thread Safety Analysis" of clang.
 * https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
 *
 * Developed based on versions from internet, with changes from us including rob, rfree.
 * @owner rob
 *
*/

#ifndef THREAD_SAFETY_ANALYSIS_MUTEX_H
#define THREAD_SAFETY_ANALYSIS_MUTEX_H

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

//#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#include <iostream>

#define _mutexshield_abort(X) do { std::cerr<<"Critical error detected in mutexes: msg: " << X << " in " \
	<< __FILE__ << ":" << __LINE__ << " must abort program NOW." << std::endl; \
	std::abort(); \
	} while(0)

// Defines an annotated interface for mutexes.
// These methods can be implemented to use any internal mutex implementation.
#include <mutex>
class CAPABILITY("mutex") Mutex {
private:
	std::mutex std_mutex;
public:
  // Acquire/lock this mutex exclusively.  Only one thread can have exclusive
  // access at any one time.  Write operations to guarded data require an
  // exclusive lock.
  void lock() ACQUIRE(){std_mutex.lock();}

  // Acquire/lock this mutex for read operations, which require only a shared
  // lock.  This assumes a multiple-reader, single writer semantics.  Multiple
  // threads may acquire the mutex simultaneously as readers, but a writer
  // must wait for all of them to release the mutex before it can acquire it
  // exclusively.
//  void ReaderLock() ACQUIRE_SHARED();

  // Release/unlock an exclusive mutex.
  void unlock() RELEASE(){std_mutex.unlock();}

  // Release/unlock a shared mutex.
//  void ReaderUnlock() RELEASE_SHARED();

  // Try to acquire the mutex.  Returns true on success, and false on failure.
  bool try_lock() TRY_ACQUIRE(true){return std_mutex.try_lock();}

  // Try to acquire the mutex for read operations.
//  bool ReaderTryLock() TRY_ACQUIRE_SHARED(true);

  // Assert that this mutex is currently held by the calling thread.
//  void AssertHeld() ASSERT_CAPABILITY(this);

  // Assert that is mutex is currently held for read operations.
//  void AssertReaderHeld() ASSERT_SHARED_CAPABILITY(this);

  // For negative capabilities.
//  const Mutex& operator!() const { return *this; }
};


// Defines an annotated interface for mutexes.
// These methods can be implemented to use any internal mutex implementation.
#include <boost/thread/shared_mutex.hpp>
class CAPABILITY("mutex") MutexShared {
private:
	boost::shared_mutex m_boost_shared_mutex;
public:
  // Acquire/lock this mutex exclusively.  Only one thread can have exclusive
  // access at any one time.  Write operations to guarded data require an
  // exclusive lock.
  void lock() ACQUIRE(){m_boost_shared_mutex.lock();}
  // Try to acquire the mutex.  Returns true on success, and false on failure.
  bool try_lock() TRY_ACQUIRE(true){return m_boost_shared_mutex.try_lock();}

  // Acquire/lock this mutex for read operations, which require only a shared
  // lock.  This assumes a multiple-reader, single writer semantics.  Multiple
  // threads may acquire the mutex simultaneously as readers, but a writer
  // must wait for all of them to release the mutex before it can acquire it
  // exclusively.
  void lock_shared() ACQUIRE_SHARED(){m_boost_shared_mutex.lock_shared();}
  // Try to acquire the mutex for read operations.
  bool try_lock_shared() TRY_ACQUIRE_SHARED(true) { return m_boost_shared_mutex.try_lock_shared(); }

  // Release/unlock an exclusive mutex.
  void unlock() RELEASE(){m_boost_shared_mutex.unlock();}

  // Release/unlock a shared mutex.
  void unlock_shared() RELEASE_SHARED(){m_boost_shared_mutex.unlock_shared();}

  // Assert that this mutex is currently held by the calling thread.
//  void AssertHeld() ASSERT_CAPABILITY(this);

  // Assert that is mutex is currently held for read operations.
//  void AssertReaderHeld() ASSERT_SHARED_CAPABILITY(this);

  // For negative capabilities.
//  const Mutex& operator!() const { return *this; }
};


// MutexLocker is an RAII class that acquires a mutex in its constructor, and
// releases it in its destructor.
template <class t_mutex>
class SCOPED_CAPABILITY LockGuard {
private:
  t_mutex &mut;

public:
  LockGuard(t_mutex &mu) ACQUIRE(mu) : mut(mu) { mut.lock(); }
  ~LockGuard() RELEASE() { mut.unlock(); }
};

template <class t_mutex>
class SCOPED_CAPABILITY UniqueLockGuardRO {
private:
  t_mutex & m_mut;
  bool m_locked; ///< not all mutex types know if they are locked apparentyl?
public:
  UniqueLockGuardRO(t_mutex &mu) noexcept ACQUIRE_SHARED(m_mut) : m_mut(mu), m_locked(true) {
  	try { m_mut.lock_shared(); } catch(...) { _mutexshield_abort("LG-RO constr"); } }
//  ~UniqueLockGuardRO() noexcept RELEASE_SHARED() { // https://stackoverflow.com/questions/33608378/clang-thread-safety-annotation-and-shared-capabilities
  ~UniqueLockGuardRO() noexcept RELEASE() {
  	try { if (m_locked) m_mut.unlock_shared(); } catch(...) { _mutexshield_abort("LG-RO destr"); } }

  void lock() noexcept ACQUIRE_SHARED(m_mut) {
  	try {
  		if (m_locked) _mutexshield_abort("LG-RO already locked");
	  	m_mut.lock_shared(); m_locked=true;
	  } catch(...) { _mutexshield_abort("LG-RO lock"); } }

//  void unlock() noexcept RELEASE_SHARED() { // https://stackoverflow.com/questions/33608378/clang-thread-safety-annotation-and-shared-capabilities
  void unlock() noexcept RELEASE() {
  	try {
  		if (!m_locked) _mutexshield_abort("LG-RO already unlocked");
	  	m_mut.unlock_shared(); m_locked=false;
	  } catch(...) { _mutexshield_abort("LG-RO unlock"); } }
};

template <class t_mutex>
class SCOPED_CAPABILITY UniqueLockGuardRW {
private:
  t_mutex & m_mut;
  bool m_locked; ///< not all mutex types know if they are locked apparentyl?
public:
  UniqueLockGuardRW(t_mutex &mu) noexcept ACQUIRE(mu) : m_mut(mu), m_locked(true) {
  	try { m_mut.lock(); } catch(...) { _mutexshield_abort("LG-RW constr"); } }
  ~UniqueLockGuardRW() noexcept RELEASE() {
  	try { if (m_locked) m_mut.unlock(); } catch(...) { _mutexshield_abort("LG-RW destr"); } }

  void lock() noexcept ACQUIRE(m_mut) {
  	try {
  		if (m_locked) _mutexshield_abort("LG-RW already locked");
	  	m_mut.lock(); m_locked=true;
	  } catch(...) { _mutexshield_abort("LG-RW lock"); } }
  void unlock() noexcept RELEASE() {
  	try {
  		if (!m_locked) _mutexshield_abort("LG-RW already unlocked");
	  	m_mut.unlock(); m_locked=false;
	  } catch(...) { _mutexshield_abort("LG-RW unlock"); } }
};

//  UniqueLockGuard(t_mutex &mu, tag_lock_exclusive) ACQUIRE_SHARED(mu) : m_unique_lock(mu,std::defer_lock) {
//  	mu->lock_shared();
// 	}


#ifdef USE_LOCK_STYLE_THREAD_SAFETY_ATTRIBUTES
// The original version of thread safety analysis the following attribute
// definitions.  These use a lock-based terminology.  They are still in use
// by existing thread safety code, and will continue to be supported.

// Deprecated.
#define PT_GUARDED_VAR \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded)

// Deprecated.
#define GUARDED_VAR \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded)

// Replaced by REQUIRES
#define EXCLUSIVE_LOCKS_REQUIRED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

// Replaced by REQUIRES_SHARED
#define SHARED_LOCKS_REQUIRED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_locks_required(__VA_ARGS__))

// Replaced by CAPABILITY
#define LOCKABLE \
  THREAD_ANNOTATION_ATTRIBUTE__(lockable)

// Replaced by SCOPED_CAPABILITY
#define SCOPED_LOCKABLE \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

// Replaced by ACQUIRE
#define EXCLUSIVE_LOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

// Replaced by ACQUIRE_SHARED
#define SHARED_LOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))

// Replaced by RELEASE and RELEASE_SHARED
#define UNLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

// Replaced by TRY_ACQUIRE
#define EXCLUSIVE_TRYLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

// Replaced by TRY_ACQUIRE_SHARED
#define SHARED_TRYLOCK_FUNCTION(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))

// Replaced by ASSERT_CAPABILITY
#define ASSERT_EXCLUSIVE_LOCK(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_exclusive_lock(__VA_ARGS__))

// Replaced by ASSERT_SHARED_CAPABILITY
#define ASSERT_SHARED_LOCK(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_lock(__VA_ARGS__))

// Replaced by EXCLUDE_CAPABILITY.
#define LOCKS_EXCLUDED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

// Replaced by RETURN_CAPABILITY
#define LOCK_RETURNED(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#endif  // USE_LOCK_STYLE_THREAD_SAFETY_ATTRIBUTES

#endif  // THREAD_SAFETY_ANALYSIS_MUTEX_H

#undef _mutexshield_abort
