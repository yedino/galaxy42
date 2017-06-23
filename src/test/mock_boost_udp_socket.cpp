#include "mock_boost_udp_socket.hpp"

using namespace mock;

bool mock_boost_udp_socket::s_is_open = true;
bool mock_boost_udp_socket::s_set_option_throws = false;

mock_boost_udp_socket::mock_boost_udp_socket(boost::asio::io_service &io_service,
                                             const boost::asio::ip::udp::endpoint &endpoint) {
	_UNUSED(io_service);
	_UNUSED(endpoint);
}

mock_boost_udp_socket::mock_boost_udp_socket(boost::asio::io_service &io_service,
                                             const boost::asio::ip::udp &protocol) {
	_UNUSED(io_service);
	_UNUSED(protocol);
}

mock_boost_udp_socket::mock_boost_udp_socket(boost::asio::io_service &io_service) {
	_UNUSED(io_service);
}

bool mock_boost_udp_socket::is_open() {
	return s_is_open;
}

void mock_boost_udp_socket::set_option(const int &) {
	if (s_set_option_throws)
		throw boost::system::system_error(boost::asio::error::fault); // example error
}
