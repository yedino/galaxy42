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

		bool m_empty; ///< if empty, then all other data is invalid and should not be used, except for e.g. m_buf and things set by Clear()

		void Clear(); ///< resets the weld to an empty container

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
class vector_mutexed_sharedptr {
	public:
		MutexShared m_mutex_all; ///< protects the entire vector
		vector< with_mutex< MutexShared, unique_ptr<TObj> > > m_data; ///< our data

		vector_mutexed_sharedptr(const vector_mutexed_sharedptr<TObj> & other) = delete;
		vector_mutexed_sharedptr(vector_mutexed_sharedptr<TObj> && other) = delete;
		vector_mutexed_sharedptr<TObj> operator=(const vector_mutexed_sharedptr<TObj> & other) = delete;
		vector_mutexed_sharedptr<TObj> operator=(vector_mutexed_sharedptr<TObj> && other) = delete;

		void grow_to(size_t size); ///< safely increase size
		template <typename TRet, typename TFun> TRet run_on(size_t ix, TFun & fun);
};

template <typename TObj>
void vector_mutexed_sharedptr<TObj>::grow_to(size_t size) {
	std::lock_guard<MutexShared> lg(m_mutex_all); // hard lock on container (avoid concurent resizes),
	// but all existing elements can be worked on in background (as the ELEMENTS address does not change),
	// provided that the other operations releases their lock on m_mutex_all only when they got the raw pointer/reference
	// and loked that
	m_data.resize(size);
}

template <typename TObj>
template <typename TRet, typename TFun>
TRet vector_mutexed_sharedptr<TObj>::run_on(size_t ix, TFun & fun) {
	TObj * object = nullptr;
	{
		std::unique_lock<MutexShared> lg(m_mutex_all, std::defer_lock);
		lg.mutex()->lock_shared(); // read lock on container
		object = m_data.at(ix).get();
		// unlocking container
	}
	return fun( object );
}

/**
 * Base of all the main engine logic, e.g. reading from tuntap, encryption, writting into cable/udp,
 * it can be later implemented in child class Galaxy - that has access to encryption, cables etc.
 */
class c_galaxysrv_engine {
	protected:
		c_galaxysrv_engine()=default;
		virtual ~c_galaxysrv_engine()=default;

	//vector< with_mutex< Mutex, shared_ptr< c_weld > > >
	//	with_mutex<Mutex,  > m_welds;
		// vector< shared_ptr<  c_emitqueue > > m_emitqueues;
};



