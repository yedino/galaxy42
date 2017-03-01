#include "transport_udp.hpp"

using namespace boost::asio;

void c_transport_udp::send_data(boost::any dest, const char *data, size_t size_of_data) {
	ip::udp::endpoint destination_endpoint;
	destination_endpoint = boost::any_cast<decltype(destination_endpoint)>(dest);
	ip::udp::socket socket(m_io_service);
	socket.send_to(buffer(data, size_of_data), destination_endpoint);
}
