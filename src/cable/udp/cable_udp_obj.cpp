#include "cable_udp_obj.hpp"
#include "cable_udp_addr.hpp"

using namespace boost::asio::ip;

void c_cable_udp::send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) {
	auto destination_endpoint = (boost::any_cast<c_cable_udp_addr::t_addr>( dest.get_addrdata() ));
	udp::socket socket(m_io_service);
	socket.send_to(boost::asio::buffer(data, size), destination_endpoint);
}

size_t c_cable_udp::receive_from(c_cable_base_addr &source, unsigned char *const data, size_t size) {
	return 0;
}

void c_cable_udp::listen_on(c_cable_base_addr &local_address) {

}
