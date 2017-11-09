#include "c_fake_tun.hpp"

using namespace boost::asio::ip;

c_fake_tun::c_fake_tun(
	boost::asio::io_service &io_service,
	const std::string listen_ipv4_address,
	const short listen_port)
:
	m_socket(io_service)
{
	auto listen_endpoint = udp::endpoint(address_v4::from_string(listen_ipv4_address), listen_port);
	m_socket.bind(listen_endpoint);
}

size_t c_fake_tun::send_to_tun(const unsigned char *data, size_t size) {
	return m_socket.send(boost::asio::buffer(data, size));
}

size_t c_fake_tun::read_from_tun(unsigned char *const data, size_t size) {
	return m_socket.receive(boost::asio::buffer(data, size));
}

