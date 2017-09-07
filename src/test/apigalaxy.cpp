
#include "gtest/gtest.h"
#include "libs1.hpp"

#include <apigalaxy/apigalaxy.hpp>

TEST(apigalaxy, is_valid_peer_reference){
	EXPECT_TRUE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107:9042)"));
	EXPECT_TRUE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5445:554e:45ba:e4da@(udp:192.168.9.107:19042)"));
	EXPECT_TRUE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5445:554e:45ba:e4da@(udp:192.168.9.107:65535)"));

	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.999:9042)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4dafff@(udp:192.168.1.107:9042)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4dx@(udp:192.168.1.107:9042)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107:65536)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107:-1)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107:)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(udp:192.168.1.107)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(tcp:192.168.1.107:9042)"));
	EXPECT_FALSE(apigalaxy::is_valid_peer_reference("fd42:e5ca:4e2a:cd13:5435:5e4e:45bf:e4da@(xyzproto:192.168.1.107:9042)"));
}
