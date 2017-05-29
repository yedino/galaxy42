#pragma once

#include <libs0.hpp>
#include <boost/asio.hpp>
#include <netbuf.hpp>
#include "stdplus/arrayvector.hpp"

#include <shared_mutex>


/**
 * [[optim-engine]] this marker in comments of code will mark things that could be written in other way,
 * that could increase speed of this engine possibly. (how ever it's up for experiments to decided is it worth it)
 * TODO
 */

/**
 * This is both a Weld, and EI (Emit Input - buffer that forms it).
 */
class c_weld {
	public:
		c_weld(size_t memsize);

		Mutex m_mutex; ///< mutex that you must own to operate on this object

		bool m_empty; ///< if empty, then all other data is invalid and should not be used, except for e.g. m_buf

		boost::asio::ip::address_v6 m_dst; ///< the destination of this BI/Weld
		std::chrono::steady_clock::time_point m_time_start; ///< time of oldest Merit here; to decide on latency on sending

		c_netbuf m_buf; ///< our buffer - the entire memory; but used with pointers
		size_t m_bufWrite; ///< write pointer to memory inside m_buf; [[optim-engine]]
		size_t m_bufRead; ///< read pointer to memory inside m_buf ; [[optim-engine]] can use pointer here
};

/**
 * Queue with the Welds to be Emited.
 * Emited from us (send from our tuntap, to next peer).
 */
class c_emitqueue {
	public:
		Mutex m_mutex; ///< mutex that you must own to operate on this object
		std::vector<  weak_ptr< c_weld > > m_welds; ///< the welds that I want to send
};

/**
 * Bag fragment represents light-weight "reference" to given weld's data in range [pos_begin .. pos_end)
 */
class c_bag_fragment {
	public:
		weak_ptr< c_weld > m_weld;
		size_t pos_begin, pos_end;
};

/*
 *
 */
class c_bag {
	public:
		static constexpr int m_max_fragments = 4;

		stdplus::arrayvector<c_bag_fragment, m_max_fragments> m_fragments;
};

/**
 * Cart (e.g. while being created as a queue of welds)
 */
class c_cart {
	public:
};

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
		TObj& get( std::lock_guard<TMutex> & ); ///< access the object, after showing that you do hold the lock
		const TObj& get( std::lock_guard<TMutex> & ) const; ///< access the object, after showing that you do hold the lock

		// std::lock_guard<TMutex> get_lock() const; ///< lock the object, save the lock outside // TODO possible?

		/// get the mutex, you should ONLY LOCK-GUARD IT, do not use it for any other purpose.
		TMutex & get_mutex_for_locking() const;

	private:
		mutable TMutex m_mutex;
		TObj m_obj GUARDED_BY( m_mutex ); ///< the object. Btw here we additionally use clang-static-analysis.
};

template <typename TMutex, typename TObj>
TObj& with_mutex<TMutex,TObj>::get( std::lock_guard<TMutex> & ) { return m_obj; }

template <typename TMutex, typename TObj>
const TObj& with_mutex<TMutex,TObj>::get( std::lock_guard<TMutex> & ) const { return m_obj; }

template <typename TMutex, typename TObj>
TMutex & with_mutex<TMutex,TObj>::get_mutex_for_locking() const { return m_mutex; }

/*
template <typename TMutex, typename TObj>
std::lock_guard<TMutex> with_mutex<TMutex,TObj>::get_lock() const {
	return std::lock_guard<TMutex>(m_mutex);
}
*/

/**
 * Base of all the main engine logic, e.g. reading from tuntap, encryption, writting into cable/udp,
 * it can be later implemented in child class Galaxy - that has access to encryption, cables etc.
 */
class c_galaxysrv_engine {
	protected:
		c_galaxysrv_engine()=default;
		virtual ~c_galaxysrv_engine()=default;

		with_mutex<MutexShared, vector< shared_ptr<  c_weld > > > m_welds;
		vector< shared_ptr<  c_emitqueue > > m_emitqueues;

};



