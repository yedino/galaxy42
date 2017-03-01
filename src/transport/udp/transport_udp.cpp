#include "transport_udp.hpp"
#include "transport_udp_addr.hpp"

using namespace boost::asio;

void c_transport_udp::send_data(boost::any dest, const unsigned char *data, size_t size_of_data) {
	c_transport_udp_addr destination_endpoint;
	destination_endpoint = boost::any_cast<decltype(destination_endpoint)>(dest);
	ip::udp::socket socket(m_io_service);
	socket.send_to(buffer(data, size_of_data), destination_endpoint.get_native());
}
