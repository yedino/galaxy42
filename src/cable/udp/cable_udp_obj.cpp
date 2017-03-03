#include "cable_udp_obj.hpp"
#include "cable_udp_addr.hpp"

using namespace boost::asio::ip;

c_cable_udp::c_cable_udp()
:
	m_io_service(),
	m_listen_socket(m_io_service)
{
}

void c_cable_udp::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	auto destination_endpoint = (boost::any_cast<c_cable_udp_addr::t_addr>( dest.get_addrdata() ));
	udp::socket socket(m_io_service);
	socket.send_to(boost::asio::buffer(data, size), destination_endpoint);
}

size_t c_cable_udp::receive_from(c_cable_base_addr &source, unsigned char *const data, size_t size) {
	udp::endpoint source_endpoint;
	size_t readed_bytes = m_listen_socket.receive_from(boost::asio::buffer(data, size), source_endpoint);
	source.init_addrdata(source_endpoint);
	return readed_bytes;
}

void c_cable_udp::listen_on(c_cable_base_addr &local_address) {
	udp::endpoint local_endpoint = boost::any_cast<c_cable_udp_addr::t_addr>(local_address.get_addrdata());
	m_listen_socket.bind(local_endpoint);
}
