
#pragma once

///@TODO@rob change it to stdplus/mutex_attributes.hpp when you would move the file there
#include "../mutex.hpp"

namespace stdplus {

/**
 * An object + it's mutex, we guarantee that you will use the object only under mutex if you use
 * it only as .get(....).something - that is unless you somehow store underlying object address/reference yourself and keep
 * using it after lock is released.
 * @owner rfree
 * @TODO with a better lock type, we could prove that the lock was taken for the mutex that we want here
 */
template <typename TMutex, typename TObj>
class with_mutex {
	public:

		with_mutex(const TObj & value); ///< construct me from value of TObj
		with_mutex(TObj && value); ///< construct me by moving value of TObj

		TObj& get( std::lock_guard<TMutex> & ); ///< access the object, after showing that you do hold the lock
		const TObj& get( std::lock_guard<TMutex> & ) const; ///< access the object, after showing that you do hold the lock
		TObj& get( std::unique_lock<TMutex> & lg ); ///< access the object, after showing that you do hold the lock
		const TObj& get( std::unique_lock<TMutex> & lg ) const; ///< access the object, after showing that you do hold the lock

		std::unique_lock<TMutex> get_lock_RO() const; ///< lock the object (shared / RO lock), if possible for the Mutex type
		std::unique_lock<TMutex> get_lock_RW() const; ///< lock the object (exclusive / RW lock), save the lock outside

	private:
		mutable TMutex m_mutex;
		TObj m_obj GUARDED_BY( m_mutex ); ///< the object. Btw here we additionally use clang-static-analysis.
};

template <typename TMutex, typename TObj>
with_mutex<TMutex,TObj>::with_mutex(const TObj & value)
: m_obj(value)
{ }

template <typename TMutex, typename TObj>
with_mutex<TMutex,TObj>::with_mutex(TObj && value)
: m_obj(std::move(value))
{ }

template <typename TMutex, typename TObj>
TObj& with_mutex<TMutex,TObj>::get( std::lock_guard<TMutex> & ) { return m_obj; }

template <typename TMutex, typename TObj>
const TObj& with_mutex<TMutex,TObj>::get( std::lock_guard<TMutex> & ) const { return m_obj; }

template <typename TMutex, typename TObj>
TObj& with_mutex<TMutex,TObj>::get( std::unique_lock<TMutex> & lg) { _chek_abort(lg.owns_lock());  return m_obj; }

template <typename TMutex, typename TObj>
const TObj& with_mutex<TMutex,TObj>::get( std::unique_lock<TMutex> & lg) const { _chek_abort(lg.owns_lock());  return m_obj; }

template <typename TMutex, typename TObj>
std::unique_lock<TMutex> with_mutex<TMutex,TObj>::get_lock_RO() const {
	return std::unique_lock<TMutex>(m_mutex);
}

template <typename TMutex, typename TObj>
std::unique_lock<TMutex> with_mutex<TMutex,TObj>::get_lock_RW() const {
	std::unique_lock<TMutex> lg(m_mutex, std::defer_lock);
	m_mutex.lock_shared();
	return lg;
}

}

