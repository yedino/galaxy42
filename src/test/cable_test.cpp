#include "gtest/gtest.h"
#include <cable/base/cable_base_addr.hpp>
#include <cable/shm/cable_shm_addr.hpp>
#include <cable/simulation/cable_simul_addr.hpp>
#include <cable/simulation/world.hpp>
#include <cable/udp/cable_udp_addr.hpp>
#include <cable/asio_ioservice_manager.hpp>
#include <iostream>

TEST(cable_Test, operators_test) {
	auto world = make_shared<c_world>();
	auto udp_addr1 = c_cable_base_addr::cable_make_addr("udp:192.166.218.58:9042");
	auto udp_addr2 = c_cable_base_addr::cable_make_addr("udp:192.166.218.59:9042");
	auto auto_as_udp_addr1 = c_cable_base_addr::cable_make_addr("auto:192.166.218.58:9042");
	auto auto_as_udp_addr2 = c_cable_base_addr::cable_make_addr("auto:192.166.218.58:9043");
	auto tcp_as_udp_addr1 = c_cable_base_addr::cable_make_addr("tcp:192.166.218.58:9042");
	auto tcp_as_udp_addr2 = c_cable_base_addr::cable_make_addr("tcp:192.165.219.88:19042");
	auto default_as_udp_addr1 = c_cable_base_addr::cable_make_addr("192.166.218.58:9042");
	auto default_as_udp_addr2 = c_cable_base_addr::cable_make_addr("92.66.18.8:1942");
	auto default_port_as_udp_addr1 = c_cable_base_addr::cable_make_addr("192.166.218.58");
	auto default_port_as_udp_addr2 = c_cable_base_addr::cable_make_addr("12.16.28.5");
	auto simul_addr = make_unique<c_cable_simul_addr>( world->generate_simul_cable() );
	auto shm_addr1 = c_cable_base_addr::cable_make_addr("shm:test1");
	auto shm_addr2 = c_cable_base_addr::cable_make_addr("shm:test1");
	auto shm_addr3 = c_cable_base_addr::cable_make_addr("shm:test2");

	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(auto_as_udp_addr1));
	EXPECT_EQ(UsePtr(tcp_as_udp_addr1), UsePtr(auto_as_udp_addr1));
	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(default_as_udp_addr1));
	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(default_port_as_udp_addr1));
	EXPECT_EQ(UsePtr(shm_addr1), UsePtr(shm_addr2));

	EXPECT_LT(UsePtr(udp_addr1), UsePtr(udp_addr2));
	EXPECT_LT(UsePtr(simul_addr), UsePtr(udp_addr2));
	EXPECT_LT(UsePtr(udp_addr1), UsePtr(auto_as_udp_addr2));
	EXPECT_LT(UsePtr(tcp_as_udp_addr2), UsePtr(udp_addr2));
	EXPECT_LT(UsePtr(default_port_as_udp_addr2), UsePtr(default_as_udp_addr2));
	EXPECT_LT(UsePtr(shm_addr1), UsePtr(default_as_udp_addr2));
	EXPECT_LT(UsePtr(shm_addr1), UsePtr(shm_addr3));
}

TEST(c_asioservice_menager_test, resize_to_at_least_test)
{
	size_t size = 1;
	c_asioservice_manager manager(size);
	size_t capacity = manager.capacity();
	EXPECT_EQ(manager.size(), size);

	manager.resize_to_at_least(size);
	EXPECT_EQ(manager.size(), size);

	manager.resize_to_at_least(size-1);
	EXPECT_EQ(manager.size(), size);

	if (size + 1 <= manager.capacity())
		size++;
	manager.resize_to_at_least(size);
	EXPECT_EQ(manager.size(), size);
	EXPECT_EQ(manager.capacity(), capacity);

	size = manager.capacity()-1;
	manager.resize_to_at_least(size);
	EXPECT_EQ(manager.size(), size);

	size = manager.capacity();
	manager.resize_to_at_least(size);
	EXPECT_EQ(manager.size(), size);
	EXPECT_THROW(manager.resize_to_at_least(manager.capacity()+1), err_check_prog);
}
