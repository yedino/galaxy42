#include "cable_udp_obj.hpp"
#include "cable_udp_addr.hpp"
#include "../asio_ioservice_manager.hpp"
#include <libs0.hpp>

using namespace boost::asio::ip;

c_cable_udp::c_cable_udp(shared_ptr<c_asioservice_manager> iomanager, const c_card_selector &source_addr)
:
	c_asiocable(iomanager),
	m_read_socket(get_io_service(), boost::asio::ip::udp::v4()),
	m_write_socket(get_io_service(), dynamic_cast<const c_cable_udp_addr&>(UsePtr(source_addr.get_my_addr())).get_addr() )
{
	_note("Created UDP card: \n"
		<< "  Read socket:  open="<< m_read_socket.is_open() << " native="<<m_read_socket.native_handle() << "\n"
		<< "  Write socket: open="<< m_write_socket.is_open() << " native="<<m_write_socket.native_handle());
}

void c_cable_udp::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	_dbg3("Seding (blocking) UDP size=" << size << " dest=" << dest);
	try {
		udp::endpoint destination_endpoint = dynamic_cast<const c_cable_udp_addr &>(dest).get_addr();
		_dbg4("UDP to " << destination_endpoint);
		m_write_socket.send_to(boost::asio::buffer(data, size), destination_endpoint);
	} catch(...) {
		_warn("Can not send UDP");
		throw;
	}
}

void c_cable_udp::async_send_to(const c_cable_base_addr &dest, const unsigned char *data, size_t size, write_handler handler) {
	_dbg3("Seding (asyncblocking) UDP size=" << size << " dest=" << dest);
	try {
		udp::endpoint destination_endpoint = dynamic_cast<const c_cable_udp_addr &>(dest).get_addr();
		m_write_socket.async_send_to(boost::asio::buffer(data, size), destination_endpoint,
			// handler cannot be moved in lambda capture list (MSVC compilation problems)
			[handler, data](const boost::system::error_code& error, std::size_t bytes_transferred) {
				_UNUSED(error);
				handler(data, bytes_transferred);
			} // lambda
		);
	} catch(...) {
		_warn("Can not send UDP");
		throw;
	}
}

size_t c_cable_udp::receive_from(c_card_selector &source, unsigned char *const data, size_t size) {
	_dbg3("Receive (blocking) UDP");
	try {
		udp::endpoint their_ep;
		size_t readed_bytes = m_read_socket.receive_from(boost::asio::buffer(data, size), their_ep);
		unique_ptr<c_cable_base_addr> their_ep_cable = make_unique<c_cable_udp_addr>( their_ep );
		source = c_card_selector( std::move( their_ep_cable ) );
		return readed_bytes;
	} catch(...) {
		_warn("Can not receive UDP");
		throw;
	}
}

void c_cable_udp::async_receive_from(unsigned char *const data, size_t size, read_handler handler) {
	_warn("this function is disabled"); // need to fix it after switch to use c_card_selector
	_UNUSED(data); _UNUSED(size); _UNUSED(handler);
	/*
	_dbg3("Receive (asyn) UDP");

	auto endpoint_iterator = [this] {
		Unique_lock<Mutex> lock(m_enpoint_list_mutex);
		m_endpoint_list.emplace_back();
		auto ret = m_endpoint_list.end();
		--ret;
		return ret;
	}();

	m_read_socket.async_receive_from(boost::asio::buffer(data, size), *endpoint_iterator,
		[this, handler, data, endpoint_iterator]
		(const boost::system::error_code& error, std::size_t bytes_transferred)
		{
			_UNUSED(error);	// TODO: handler error - do not run handler then?
			std::unique_ptr<c_cable_base_addr> source_addr_cable = std::make_unique<c_cable_udp_addr>( *endpoint_iterator );
			Unique_lock<Mutex> lock(m_enpoint_list_mutex);
			m_endpoint_list.erase(endpoint_iterator);
			lock.unlock();

			handler(data, bytes_transferred, std::move(source_addr_cable));
		} // lambda
	);
	*/
}

void c_cable_udp::listen_on(const c_card_selector & local_address) {
	_fact("Listen on " << local_address );
	udp::endpoint local_endpoint = dynamic_cast<const c_cable_udp_addr &>(UsePtr(local_address.get_my_addr())).get_addr();
	m_read_socket.bind(local_endpoint);
	_goal("Listening on " << local_endpoint );
}

void c_cable_udp::stop_threadsafe() {
	_note("Stoping socket");
	boost::system::error_code ec;
	m_read_socket.shutdown( boost::asio::ip::udp::socket::shutdown_both , ec);
	_dbg1("Shutdown ec="<<ec);
	m_read_socket.close();
	m_write_socket.shutdown( boost::asio::ip::udp::socket::shutdown_both , ec);
	_dbg1("Shutdown ec="<<ec);
	m_write_socket.close();
}

