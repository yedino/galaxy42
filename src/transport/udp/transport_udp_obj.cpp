#include "transport_udp_obj.hpp"
#include "transport_udp_addr.hpp"

using namespace boost::asio;

void c_transport_udp::send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) {
	auto destination_endpoint = (boost::any_cast<c_transport_udp_addr::t_addr>( dest.get_addrdata() ));
	ip::udp::socket socket(m_io_service);
	socket.send_to(buffer(data, size), destination_endpoint);
}
