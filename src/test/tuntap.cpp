#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../tuntap/linux/c_tuntap_linux_obj.hpp"

using testing::Return;
using testing::_;
using testing::Invoke;

TEST(tuntap, bad_open) {
	EXPECT_ANY_THROW(c_tuntap_linux_obj tuntap);
}

TEST(tuntap, send_to_tun) {
	c_tuntap_linux_obj tuntap;
	// normal write to tun
	EXPECT_CALL(tuntap.m_tun_stream, write_some(_)).WillRepeatedly(testing::Invoke(
		[](const boost::asio::const_buffers_1 &buffers) {
			return boost::asio::buffer_size(buffers);
	}));

	std::array<unsigned char, 3> buff1;
	std::array<int, 5> buff2;
	std::array<long int, 8> buff3;
	EXPECT_EQ(tuntap.send_to_tun(buff1.data(), buff1.size()), buff1.size());
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff2.data()), buff2.size() * sizeof(decltype(buff2)::value_type)),
	          buff2.size() * sizeof(decltype(buff2)::value_type));
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff3.data()), buff3.size() * sizeof(decltype(buff3)::value_type)),
	          buff3.size() * sizeof(decltype(buff3)::value_type));

}

TEST(tuntap, send_to_tun_with_error) {
	c_tuntap_linux_obj tuntap;
	EXPECT_CALL(tuntap.m_tun_stream, write_some(_)).WillRepeatedly(testing::Invoke(
		[](const boost::asio::const_buffers_1 &buffers) -> size_t {
			// write_some can throw
			// http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/reference/posix__basic_stream_descriptor/write_some/overload1.html
			_UNUSED(buffers);
			throw boost::system::system_error(boost::asio::error::eof);
	}));

	std::array<unsigned char, 3> buff1;
	std::array<int, 5> buff2;
	std::array<long int, 8> buff3;
	EXPECT_THROW(tuntap.send_to_tun(buff1.data(), buff1.size()),
	             boost::system::system_error);
	EXPECT_THROW(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff2.data()), buff2.size() * sizeof(decltype(buff2)::value_type)),
	             boost::system::system_error);
	EXPECT_THROW(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff3.data()), buff3.size() * sizeof(decltype(buff3)::value_type)),
	             boost::system::system_error);
}
