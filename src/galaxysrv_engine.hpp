#pragma once

#include <libs0.hpp>
#include <boost/asio.hpp>
#include <netbuf.hpp>
#include "stdplus/arrayvector.hpp"


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
		Mutex m_mutex; ///< mutex that you must own to operate on this object

		bool m_empty; ///< if empty, then all other data is invalid and should not be used

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
		std::mutex m_mutex; ///< mutex that you must own to operate on this object
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
 * Base of all the main engine logic, e.g. reading from tuntap, encryption, writting into cable/udp,
 * it can be later implemented in child class Galaxy - that has access to encryption, cables etc.
 */
class c_galaxysrv_engine {
	protected:
		c_galaxysrv_engine()=default;
		virtual ~c_galaxysrv_engine()=default;

		vector< shared_ptr<  c_weld > > m_welds;
		vector< shared_ptr<  c_emitqueue > > m_emitqueues;

};



