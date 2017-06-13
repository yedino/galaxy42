#ifndef MOCK_BOOST_UDP_SOCKET_HPP
#define MOCK_BOOST_UDP_SOCKET_HPP

#include "../../depends/googletest/googlemock/include/gmock/gmock.h"
#include <boost/asio.hpp>
#include <libs0.hpp>

namespace mock {

class mock_boost_udp_socket final {
	public:
		mock_boost_udp_socket(boost::asio::io_service &io_service,
		                      const boost::asio::ip::udp::endpoint &endpoint);
		mock_boost_udp_socket(boost::asio::io_service &io_service,
		                      const boost::asio::ip::udp &protocol);
		mock_boost_udp_socket(boost::asio::io_service &io_service);
		bool is_open();
		void set_option(const int &);
		MOCK_METHOD2(send_to, std::size_t(const boost::asio::const_buffer &,
		             const boost::asio::ip::udp::endpoint &));
		MOCK_METHOD2(send_to, std::size_t(const std::vector< boost::asio::const_buffer> &,
		             const boost::asio::ip::udp::endpoint &));
		MOCK_METHOD3(async_send_to, void(
		                                 const boost::asio::const_buffer &,
		                                 const boost::asio::ip::udp::endpoint &endpoint,
		                                 std::function<void(const boost::system::error_code&, size_t)>));
		MOCK_METHOD2(receive_from, std::size_t(
		                                       const boost::asio::mutable_buffer &,
		                                       boost::asio::ip::udp::endpoint &));
		MOCK_METHOD0(native_handle, int());
		MOCK_METHOD3(async_receive_from, void(
		                                      const boost::asio::mutable_buffer &,
		                                      boost::asio::ip::udp::endpoint &,
		                                      std::function<void(const boost::system::error_code&, size_t)>));
		MOCK_METHOD1(bind, void(const boost::asio::ip::udp::endpoint &));
		MOCK_METHOD2(shutdown, void(boost::asio::ip::udp::socket::shutdown_type, boost::system::error_code &));
		MOCK_METHOD0(close, void());
		static bool s_is_open;
		static bool s_set_option_throws;
};

} // namespace

#endif // MOCK_BOOST_UDP_SOCKET_HPP
