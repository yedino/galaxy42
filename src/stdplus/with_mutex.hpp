
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
		/// Constructors and assign/move - based on myself
		///@{

		/// can NOT copy myself (anyway would be deleted, e.g. because my mutex is not copyable)
		with_mutex(const with_mutex<TMutex,TObj> & brothe) = delete;
		/// construct by moving other objecet (e.g. for vector's resize), it locks the brother while we move
		with_mutex(with_mutex<TMutex,TObj> && brother) noexcept;
		/// }@

		/// Constructors and assign/move - based on contained object TObj
		/// @{
		with_mutex(const TObj & value); ///< construct me from value of TObj
		with_mutex(TObj && value) noexcept; ///< construct me by moving value of TObj

		with_mutex<TMutex,TObj> operator=(TObj && value) noexcept; ///< move this value into me
		/// }@

		/// lock the object (shared / RO lock) for reading it, caller should save the lock outside and use for .get()
		UniqueLockGuardRO<TMutex> get_lock_RO() const;

		/// lock the object (exclusive / RW lock), caller should save the lock outside and use for .get()
		// The object must be  non-const object, because otherwise you would not be able to modify it anyway
		// so you do not need RW lock
		UniqueLockGuardRW<TMutex> get_lock_RW();

		/// access the object (RW write), after you show that you do hold the RW lock
		TObj& get( UniqueLockGuardRW<TMutex> & );

		/// access the object (RO read only), after you show that you have RO (shared) lock
		const TObj& get( UniqueLockGuardRO<TMutex> & ) const;

	private:
		mutable TMutex m_mutex;
		TObj m_obj GUARDED_BY( m_mutex ); ///< the object. Btw here we additionally use clang-static-analysis.
};

template <typename TMutex, typename TObj>
with_mutex<TMutex,TObj>::with_mutex(with_mutex<TMutex,TObj> && brother) noexcept
{
	UniqueLockGuardRW<TMutex> lg( brother.m_mutex );
	m_obj = std::move( brother.m_obj );
}

template <typename TMutex, typename TObj>
with_mutex<TMutex,TObj>::with_mutex(const TObj & value)
: m_obj(value)
{ }

template <typename TMutex, typename TObj>
with_mutex<TMutex,TObj>::with_mutex(TObj && value) noexcept
: m_obj(std::move(value))
{ }


template <typename TMutex, typename TObj>
UniqueLockGuardRO<TMutex> with_mutex<TMutex,TObj>::get_lock_RO() const {
	UniqueLockGuardRO<TMutex> lg(m_mutex);
	return lg;
}

template <typename TMutex, typename TObj>
UniqueLockGuardRW<TMutex> with_mutex<TMutex,TObj>::get_lock_RW() {
	UniqueLockGuardRW<TMutex> lg(m_mutex);
	return lg;
}

template <typename TMutex, typename TObj>
TObj& with_mutex<TMutex,TObj>::get( UniqueLockGuardRW<TMutex> & ) {
	return m_obj;
}

template <typename TMutex, typename TObj>
const TObj& with_mutex<TMutex,TObj>::get( UniqueLockGuardRO<TMutex> & ) const {
	return m_obj;
}

}

