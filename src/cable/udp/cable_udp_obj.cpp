#include "cable_udp_obj.hpp"
#include "cable_udp_addr.hpp"
#include "../asio_ioservice_manager.hpp"
#include <libs0.hpp>

#define ANTINET_socket_use_two_and_reuse

using namespace boost::asio::ip;

c_cable_udp::c_cable_udp(shared_ptr<c_asioservice_manager_base> & iomanager, const c_card_selector_base &source_addr)
:
	c_asiocable(iomanager)
	,m_multisocket_kind( default_multisocket_kind() )
	,m_write_socket( get_io_service(), (dynamic_cast<const c_cable_udp_addr&>(source_addr.get_my_addr())).get_addr() )
	#ifdef ANTINET_socket_use_two_and_reuse
	,m_read_socket( get_io_service(), boost::asio::ip::udp::v4() )
	#else
	,m_read_socket( get_io_service() )
	#endif
{
	#ifdef ANTINET_socket_use_two_and_reuse
		_note("Will set socket reuse");
		boost::asio::socket_base::reuse_address option(true);
		m_write_socket.set_option(option); // not so important when we have just 1 socket
		// if faliture throws boost::system::system_error(child class of std::runtime_error)
	#endif

	#ifdef ANTINET_socket_use_two_and_reuse
	if (!m_read_socket.is_open() || !m_write_socket.is_open())
	#else
	if (!m_read_socket.is_open())
	#endif
		throw std::runtime_error("UDP socket not open");
	_note("Created UDP card: \n"
		<< "  Read socket:  open="<< m_read_socket.is_open() << " native="<<m_read_socket.native_handle() << "\n"
		<< "  Write socket: open="<< m_write_socket.is_open() << " native="<<m_write_socket.native_handle());
}

void c_cable_udp::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	_dbg3("Seding (blocking) UDP size=" << size << " dest=" << dest);
	try {
		// convert into udp::endpoint
		udp::endpoint destination_endpoint = dynamic_cast<const c_cable_udp_addr &>(dest).get_addr();
		_dbg4("UDP to " << destination_endpoint);
		m_write_socket.send_to(boost::asio::buffer(data, size), destination_endpoint);
	} catch (const std::bad_cast &) { // can be throw by dynamic_cast
		throw std::invalid_argument("bad dest parameter type");
	} catch(...) {
		_warn("Can not send UDP");
		throw;
	}
}

void c_cable_udp::send_to(const c_cable_base_addr & dest, const t_asio_buffers_send & buffers) {
	_dbg3("Seding (blocking) UDP, buffers count = " << buffers.size() << " dest = " << dest);
	try {
		// convert into udp::endpoint
		udp::endpoint destination_endpoint = dynamic_cast<const c_cable_udp_addr &>(dest).get_addr();
		_dbg4("UDP to " << destination_endpoint);
		m_write_socket.send_to(buffers, destination_endpoint);
	} catch (const std::bad_cast &) { // can be throw by dynamic_cast
		throw std::invalid_argument("bad dest parameter type");
	} catch(...) {
		_warn("Can not send UDP");
		throw;
	}
}

void c_cable_udp::async_send_to(const c_cable_base_addr &dest, const unsigned char *data, size_t size, write_handler handler) {
	_dbg3("Seding (asyncblocking) UDP size=" << size << " dest=" << dest);
	try {
		// convert into udp::endpoint
		udp::endpoint destination_endpoint = dynamic_cast<const c_cable_udp_addr &>(dest).get_addr();
		m_write_socket.async_send_to(boost::asio::buffer(data, size), destination_endpoint,
			// handler cannot be moved in lambda capture list (MSVC compilation problems)
			[handler, data](const boost::system::error_code& error, std::size_t bytes_transferred) {
				_UNUSED(error); // ignore error because bytes_transferred == 0
				handler(data, bytes_transferred); // will be called after data write
			} // lambda
		);
	} catch (const std::bad_cast &) { // can be throw by dynamic_cast
		throw std::invalid_argument("bad dest parameter type");
	} catch(...) {
		_warn("Can not send UDP");
		throw;
	}
}

size_t c_cable_udp::receive_from(c_card_selector_base &source, unsigned char *const data, size_t size) {
	_dbg3("Receive (blocking) UDP");
	try {
		udp::endpoint their_ep; // needed for asio function
		// read data from one of sockets
		size_t readed_bytes = (has_separate_rw() ? m_read_socket : m_write_socket).receive_from(boost::asio::buffer(data, size), their_ep);
		unique_ptr<c_cable_base_addr> their_ep_cable = make_unique<c_cable_udp_addr>( their_ep ); // create new address object
		dynamic_cast<t_selector_type&>(source) = t_selector_type( std::move( their_ep_cable ) ); // write into out reference
		return readed_bytes;
	} catch(...) {
		_warn("Can not receive UDP");
		throw;
	}
}

size_t c_cable_udp::receive_from(c_cable_base_addr &source, unsigned char *const data, size_t size) {
	_dbg3("Receive (blocking) UDP");
	try {
		_UNUSED(dynamic_cast<c_cable_udp_addr&>(source)); // check type
		udp::endpoint their_ep; // needed for asio function
		size_t readed_bytes = (has_separate_rw() ? m_read_socket : m_write_socket).receive_from(boost::asio::buffer(data, size), their_ep);
		c_cable_udp_addr their_ep_addr( their_ep );
		dynamic_cast<c_cable_udp_addr&>(source) = their_ep_addr;
		return readed_bytes;
	} catch (const std::bad_cast &) { // can be throw by dynamic_cast
		throw std::invalid_argument("bad dest parameter type");
	} catch ( ... ) {
		_warn("Can not receive UDP");
		throw;
	}
}

void c_cable_udp::async_receive_from(unsigned char *const data, size_t size, read_handler handler) {
	_dbg3("Receive (asyn) UDP");

	// endpoint_iterator is iterator to last element in m_endpoint_list
	// iterator must be valid until the handler is called
	auto endpoint_iterator = [this] {
		LockGuard<Mutex> lock(m_enpoint_list_mutex);
		m_endpoint_list.emplace_back(); // create new last element
		auto ret = m_endpoint_list.end();
		--ret; // because ret points to past-the-last element
		return ret;
	}(); // lamnda end and call

	m_read_socket.async_receive_from(boost::asio::buffer(data, size), *endpoint_iterator,
		[this, handler, data, endpoint_iterator]
		(const boost::system::error_code& error, std::size_t bytes_transferred)
		{
			_UNUSED(error); // captured handler should by always called, if error bytes_transferred == 0 so ignore error variable
			std::unique_ptr<c_cable_base_addr> source_addr_cable = std::make_unique<c_cable_udp_addr>( *endpoint_iterator );
			UniqueLockGuardRW<Mutex>lock(m_enpoint_list_mutex);
			m_endpoint_list.erase(endpoint_iterator);
			lock.unlock();
			c_card_selector selector(std::move(source_addr_cable));
			handler(data, bytes_transferred, selector);
		} // lambda
	);
}

bool c_cable_udp::has_separate_rw() const noexcept { ///< do we use separate read and write socket, or is this the same socket
	switch (m_multisocket_kind) {
		case (t_multisocket_kind::one_rw): return true; break;
		case (t_multisocket_kind::separate_rw): return false; break;
		default: break;
	}
	_check(false);
	return false;
}

t_multisocket_kind c_cable_udp::default_multisocket_kind() {
	return t_multisocket_kind::separate_rw;
}


void c_cable_udp::listen_on(const c_card_selector & local_address) {
	_fact("Listen on " << local_address );
	if (has_separate_rw()) {
		udp::endpoint local_endpoint = (dynamic_cast<const c_cable_udp_addr &>(local_address.get_my_addr())).get_addr();
		_info("Endpoint created: " << local_endpoint);
		_info("Binding...");
		( m_read_socket ).bind(local_endpoint);
		_goal("Listening on " << local_endpoint );
	} else _goal("The write socket already is binded (as host-address) - it is listening");
}

void c_cable_udp::set_sockopt_timeout(std::chrono::microseconds timeout) {
	set_timeout_for_socket(timeout, m_read_socket);
	set_timeout_for_socket(timeout, m_write_socket);
}

#ifdef ANTINET_socket_sockopt
void c_cable_udp::set_timeout_for_socket(std::chrono::microseconds timeout, t_socket_type &socket) {
	// ideas from http://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
	t_native_socket sys_handler = socket.native_handle();
	auto us = timeout.count();
	struct timeval timespec;
	timespec.tv_sec  = us / (1000*1000);
	timespec.tv_usec = us % (1000*1000);
	_dbg1("Setting timeout on system handler="<<sys_handler<<" to seconds " << timespec.tv_sec << " + " << timespec.tv_usec << " micro");
	int ret = setsockopt( sys_handler , SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&timespec), sizeof(timespec) );
	auto ret_errno = errno;
	if (ret != 0) {
		_warn("Can not set timeout on sys_handler="<<sys_handler<<" ret="<<ret<<" errno="<<ret_errno);
		_throw_error_runtime("Can not set timeout on sys_handler");
	}
}

#elif defined ANTINET_cancelio
void c_cable_udp::set_timeout_for_socket(std::chrono::microseconds timeout, t_socket_type &socket) {
	t_native_socket sys_handler = socket.native_handle();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
	DWORD ms_dword = boost::numeric_cast<DWORD>(ms);
	int ret = setsockopt( sys_handler , SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms_dword), sizeof(ms_dword) );
	if (ret != 0) {
		int ret_errno = WSAGetLastError();
		_warn("Can not set timeout on sys_handler" << " ret=" << ret << " WSAGetLastError=" << ret_errno);
		_throw_error_runtime("Can not set timeout on sys_handler");
	}
}
#else
	#error "ANTINET_socket_sockopt and ANTINET_cancelio not defined"
#endif

void c_cable_udp::stop_threadsafe() {
	_note("Stoping socket");

	boost::system::error_code ec;

	if (has_separate_rw()) {
		m_read_socket.shutdown( boost::asio::ip::udp::socket::shutdown_both , ec);
		_dbg1("Shutdown ec="<<ec);
		m_read_socket.close();
	}

	m_write_socket.shutdown( boost::asio::ip::udp::socket::shutdown_both , ec);
	_dbg1("Shutdown ec="<<ec);
	m_write_socket.close();
}
