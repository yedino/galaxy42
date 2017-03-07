#pragma once
#ifndef cable_ASIOCABLE_HPP
#define cable_ASIOCABLE_HPP

#include "base/cable_base_obj.hpp"
#include "asio_ioservice_manager.hpp"
#include <boost/asio.hpp>
#include "libs0.hpp"

/// This is middle class, for child classes that want to use an asio::io_service,
/// give us the asio_ioservice_manager you want to use,
/// and later use our .get_io_service()
class c_asiocable : public c_cable_base_obj {
	protected:
		c_asiocable(shared_ptr< c_asioservice_manager > & iomanager);

		boost::asio::io_service & get_io_service() { return m_io_service; }

	private:
		boost::asio::io_service & m_io_service; ///< ref. to my io_service, e.g. received from get_next_ioservice()

		/// I am holding my ioservice-manager, so that it will not die while I am alive,
		/// so therefore also mine #m_io_service remains valid
		shared_ptr< c_asioservice_manager > m_iomanager;
};

#endif // cable_UDP_HPP
