#include "mock_boost_udp_socket.hpp"

using namespace mock;

mock_boost_udp_socket::mock_boost_udp_socket(boost::asio::io_service &io_service,
                                             const boost::asio::ip::udp::endpoint &endpoint) {
	_UNUSED(io_service);
	_UNUSED(endpoint);
}
