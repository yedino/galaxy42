#include "c_udp_wrapper.hpp"
#include "c_tnetdbg.hpp"

#ifdef __linux__
c_udp_wrapper_linux::c_udp_wrapper_linux(const int listen_port)
:
	m_socket(socket(AF_INET, SOCK_DGRAM, 0))
{
	_assert(m_socket >= 0);
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(listen_port);
	int bind_result = -1;
	if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
		sockaddr_in addr4 = address_for_sock.get_ip4();
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
	}
	else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
		sockaddr_in6 addr6 = address_for_sock.get_ip6();
		bind_result = bind(m_socket, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
	}
		_assert( bind_result >= 0 ); // TODO change to except
		_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
}

void c_udp_wrapper_linux::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	auto dst_ip4 = dst_address.get_ip4(); // ip of proper type, as local variable
	sendto(m_socket, data, size_of_data, 0, reinterpret_cast<sockaddr*>(&dst_ip4), sizeof(sockaddr_in));
}

size_t c_udp_wrapper_linux::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	sockaddr_in6 from_addr_raw; // peering address of peer (socket sender), raw format
	socklen_t from_addr_raw_size = sizeof(from_addr_raw); // ^ size of it
	auto size_read = recvfrom(m_socket, data_buf, data_buf_size, 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
	if (from_addr_raw_size == sizeof(sockaddr_in6)) { // the message arrive from IP pasted into sockaddr_in6 format
		_erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
		// trivial
	}
	else if (from_addr_raw_size == sizeof(sockaddr_in)) { // the message arrive from IP pasted into sockaddr_in (ipv4) format
		sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
		from_address.set_ip4(addr);
	} else {
		_throw_error( std::runtime_error("Data arrived from unknown socket address type") );
	}
	return size_read;
}

int c_udp_wrapper_linux::get_socket() {
	return m_socket;
}

#else

c_udp_wrapper_empty::c_udp_wrapper_empty(const int listen_port) {
	_UNUSED(listen_port);
}

void c_udp_wrapper_empty::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	_UNUSED(dst_address);
	_UNUSED(data);
	_UNUSED(size_of_data);
}

size_t c_udp_wrapper_empty::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	_UNUSED(data_buf);
	_UNUSED(data_buf_size);
	_UNUSED(from_address);
	return 0;
}

#endif // __linux__

#if defined(_WIN32) || defined(__CYGWIN__)
#include <boost/bind.hpp>
c_udp_wrapper_windows::c_udp_wrapper_windows(const int listen_port)
:
	m_socket(m_io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)),
	m_bytes_readed(0)
{
	//m_socket.async_receive_from(boost::asio::buffer(m_buffer),
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_windows::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void c_udp_wrapper_windows::send_data(const c_ip46_addr &dst_address, const void *data, size_t size_of_data) {
	m_socket.send_to( // blocks
			boost::asio::buffer(data, size_of_data),
			boost::asio::ip::udp::endpoint(dst_address.get_address(), dst_address.get_assign_port())
	);
}

size_t c_udp_wrapper_windows::receive_data(void *data_buf, const size_t data_buf_size, c_ip46_addr &from_address) {
	if (m_bytes_readed > 0) { // readed data in m_buffer
		assert(data_buf_size >= m_bytes_readed);
//		std::copy_n(m_buffer.begin(), m_bytes_readed, data_buf);
		std::copy_n(&m_buffer[0], m_bytes_readed, reinterpret_cast<uint8_t *>(data_buf));
		size_t ret = m_bytes_readed;
		m_bytes_readed = 0;
		return ret;
	}
	return 0;
}

void c_udp_wrapper_windows::read_handle(const boost::system::error_code& error, size_t bytes_transferred) {
	assert(m_bytes_readed == 0);
	m_bytes_readed = bytes_transferred;
	m_socket.async_receive_from(boost::asio::buffer(m_buffer), m_sender_endpoint,
			boost::bind(&c_udp_wrapper_windows::read_handle, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

#endif // _WIN32