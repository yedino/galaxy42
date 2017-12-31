
#pragma once

/// @TODO@rob change it to stdplus/mutex_attributes.hpp when you would move the file there
#include "../mutex.hpp"

#include "stdplus/with_mutex.hpp"

#include <vector>
#include <memory>

#include "tnetdbg.hpp"


/// Class for reporting when no matching object was found in container,
/// e.g. in vector_mutexed_obj
class error_no_match_found : public std::exception { public:	const char * what() const noexcept override; };

/// Class for reporting when not enough matching objects was found in container,
/// e.g. in vector_mutexed_obj
class error_not_enough_match_found : public std::exception { public:	const char * what() const noexcept override; };

/**
 * Vector of elements that are individually locked each, and that allows for safe resizing, and safe operating on elements
 * Main operations:
 * - Access and work on any random element by it's index.
 * - Search elements checking a condition (under weak lock), if matched then hard lock and work on this element.
 * - Easily grow entire vector (increase size), even if others are working on existing elements.
 * - Shrink entire vecotr (decrease size) - e.g. by locking first the elements to be erased.
 * There can be many threads running the above operations concurently.
 * @owner rfree
 */
template <typename TObj>
class vector_mutexed_obj {
	public:

		using t_one_with_mutex = stdplus::with_mutex< MutexShared, TObj >; ///< one-element (both object & it's own mutex)
		using t_one_with_mutex_ptr = std::unique_ptr< t_one_with_mutex>; ///< pointer to one-element

		vector_mutexed_obj() = default;

		vector_mutexed_obj(const vector_mutexed_obj<TObj> & other) = delete;
		vector_mutexed_obj<TObj> operator=(const vector_mutexed_obj<TObj> & other) = delete;

		void grow_to(size_t size); ///< safely increase size
		template <typename TRet, typename TFun> TRet run_on(size_t ix, TFun & fun);

		void push_back(const TObj & obj); ///< allocate a copy of the object
		void push_back(TObj && obj); ///< allocate one-element, move given TObj rref into it
//		void push_back(t_one_with_mutex_ptr && obj_ptr); ///< move this unique-ptr one-element into us (append at end)

		/// Searches through the container until we find element for which given #fun_test function returns true,
		/// then on it we run function #fun_run - this is done #how_many times, and the result of last execution of
		/// fun_run is returned
		template <typename TRet, typename TFunTest, typename TFunRun>
		TRet run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t limit_matched=1);

		size_t size() const; ///< current size

	private:
		mutable MutexShared m_mutex_all; ///< protects the entire vector; mutable for size() etc
		std::vector< t_one_with_mutex_ptr > m_data; ///< our data
};

template <typename TObj>
void vector_mutexed_obj<TObj>::grow_to(size_t size) {
	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes),
	// but all existing elements can be worked on in background (as the ELEMENTS address does not change),
	// provided that the other operations releases their lock on m_mutex_all only when they got the raw pointer/reference
	// and loked that
	if (!(size > m_data.size())) return; // nothing to do
	m_data.resize(size);
}

template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(const TObj & obj) {
	auto new_element = std::make_unique< t_one_with_mutex >( obj ); // copy
	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments
	m_data.push_back(std::move(new_element)); // allocate a copy, and push it
	// @TODO@rfree bad. look at other push_back as example
}


template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(TObj && obj) {
	auto new_element = std::make_unique< t_one_with_mutex >( std::move(obj) );

	UniqueLockGuardRW<MutexShared> lg_all(m_mutex_all);
	// here TObj's move-constructor is used (stdplus::with_mutex forwards it)
	m_data.push_back( std::move(new_element) );
}

/*
template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(std::unique_ptr<TObj> && obj_ptr) {
	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments

	auto new_obj_with_mutex = std::make_unique< stdplus::with_mutex<MutexShared, TObj > >( std::move(obj_ptr) );

	m_data.push_back( new_obj_with_mutex );
}
*/

template <typename TObj>
template <typename TRet, typename TFun>
TRet vector_mutexed_obj<TObj>::run_on(size_t ix, TFun & fun) {
	t_one_with_mutex * one_object_with_mutex = nullptr;
	{
		UniqueLockGuardRO<MutexShared> lg_all(m_mutex_all);
		one_object_with_mutex = m_data.at(ix).get();
		// unlocking container
	}

	{
		auto &mutex = one_object_with_mutex->get_mutex();
		UniqueLockGuardRW<MutexShared> lg_one(mutex);
		return fun( one_object_with_mutex->get( lg_one ) );
	}
}

/// TODO@rob
/// bool fun_test(const TObj &)
/// anything fun_run(const TObj &) ... or lambda (catured args)
/// TRet void ? - if void then "return ;"
/// TRet -> result_of....
template <typename TObj>
template <typename TRet, typename TFunTest, typename TFunRun>
TRet vector_mutexed_obj<TObj>::run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t limit_matched) {
	_dbg2("Start of func...");
	size_t so_far=0; // how many objects matched so far
	bool is_not_enough = false;

	{
		_dbg4("Taking first RO lock");
		_dbg3("Taking first RO lock");
		_dbg2("Taking first RO lock");
		UniqueLockGuardRO<MutexShared> lg_all(m_mutex_all);
		_dbg3("Size of container m_data: " << m_data.size());
		for (auto iter = m_data.begin(); iter<m_data.end(); ++iter) {
			t_one_with_mutex & one_object_with_mutex = * iter->get();
			const TObj * obj_ro = nullptr; // pointer instead ref - because we access it in both blocks of lock-guard/RAII
			bool matched=false;
			{ // RO look and test one object
				// auto lg_one_RO( one_object_with_mutex.get_lock_RO() ); // lock RO for test
				_dbg3("will test object...");
				UniqueLockGuardRO<MutexShared> lg_one_RO( one_object_with_mutex.get_mutex() ); // lock RO for test

				obj_ro = & one_object_with_mutex.get( lg_one_RO );
				_dbg3("will test object at " << static_cast<const void*>(obj_ro) << "...");
				matched = fun_test( *obj_ro ); // <--- run the test
				_dbg3("will test object at " << static_cast<const void*>(obj_ro) << "... matched=" << matched);
				// unlock the one object
			}

			// no lock here, for a second - TODO

			if (matched) { // test succeeded - use the object
				// auto lg_one_RW( one_object_with_mutex.get_lock_RW() ); // lock RW this time // XXX TRY
				UniqueLockGuardRW<MutexShared> lg_one_RW( one_object_with_mutex.get_mutex() ); // lock RW this time
				_dbg3("will test AGAIN object at " << static_cast<const void*>(obj_ro) << "...");
				bool matched_again = fun_test( *obj_ro ); // <--- run the test, AGAIN, it could have change while not locked
				_dbg3("will test object at " << static_cast<const void*>(obj_ro) << "... matched_again=" << matched_again);
				if (matched_again) { // really matched - we know this since we took EXCLUSIVE RW lock, so it must be true still
					TObj & obj_rw = one_object_with_mutex.get( lg_one_RW );
					++so_far;
					if (so_far >= limit_matched) {
						_dbg2("running (and returning) on object: " << static_cast<const void*>( & obj_rw));
						return fun_run( obj_rw ); // <--- run the modifier (and return, it's the last one)
					} else {
						_dbg2("running on object: " << static_cast<const void*>( & obj_rw));
						fun_run( obj_rw ); // <--- run the modifier
						is_not_enough = true;
					}
				} // matched_again
				// unlock the one object
			}
		} // test all objects in loop
		if (is_not_enough) {
			_dbg2("End of func - not enough match");
			throw error_not_enough_match_found();
		}
		// unlocking container
	} // access container

	_dbg2("End of func - no match");
	throw error_no_match_found();
}

template <typename TObj>
size_t vector_mutexed_obj<TObj>::size() const {
	UniqueLockGuardRO<MutexShared> lg_all(m_mutex_all);
	return m_data.size();
}


