
#include "gtest/gtest.h"
#include <boost/asio.hpp>
#include "../utils/boost_asio_helper.hpp"
#include <libs0.hpp>

TEST(boost_asio_helper, make_ipv6_address_from_bytes) {
	boost::asio::ip::address_v6 addr1{ boost::asio::ip::address_v6::from_string("fd42:9288:db10:f1a8:29ee:28bf:17bf:a630") };

	auto addr2bytes = std::array<unsigned char,16>{ { 0xfd,0x42, 0x92,0x88, 0xdb,0x10, 0xf1,0xa8,
	0x29,0xee, 0x28,0xbf, 0x17,0xbf, 0xa6,0x30 } };

	addr1.from_string("fd42:9288:db10:f1a8:29ee:28bf:17bf:a630");

	boost::asio::ip::address_v6 addr2{ make_ipv6_address( addr2bytes ) };

	EXPECT_EQ(addr1, addr2);
}

