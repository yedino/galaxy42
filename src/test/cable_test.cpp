#include "gtest/gtest.h"
#include <cable/base/cable_base_addr.hpp>
#include <cable/shm/cable_shm_addr.hpp>
#include <cable/simulation/cable_simul_addr.hpp>
#include <cable/simulation/world.hpp>
#include <cable/udp/cable_udp_addr.hpp>
#include <cable/asio_ioservice_manager.hpp>
#include <cable/selector.hpp>
#include <iostream>
#include <tnetdbg.hpp>

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

	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(udp_addr1));

	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(auto_as_udp_addr1));
	EXPECT_EQ(UsePtr(tcp_as_udp_addr1), UsePtr(auto_as_udp_addr1));
	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(default_as_udp_addr1));
	EXPECT_EQ(UsePtr(udp_addr1), UsePtr(default_port_as_udp_addr1));
	EXPECT_EQ(UsePtr(shm_addr1), UsePtr(shm_addr2));

	EXPECT_LT(UsePtr(udp_addr1), UsePtr(udp_addr2));
	bool less1 = UsePtr(udp_addr1) < UsePtr(udp_addr2) ;
	bool less2 = UsePtr(udp_addr2) < UsePtr(udp_addr1) ;
	bool eq = UsePtr(udp_addr2) == UsePtr(udp_addr1) ;
	EXPECT_TRUE(less1);
	EXPECT_FALSE(less2);
	EXPECT_FALSE(eq);

	EXPECT_FALSE( UsePtr(udp_addr1) < UsePtr(udp_addr1) );

	EXPECT_LT(UsePtr(simul_addr), UsePtr(udp_addr2));
	EXPECT_LT(UsePtr(udp_addr1), UsePtr(auto_as_udp_addr2));
	EXPECT_LT(UsePtr(tcp_as_udp_addr2), UsePtr(udp_addr2));
	EXPECT_LT(UsePtr(default_port_as_udp_addr2), UsePtr(default_as_udp_addr2));
	EXPECT_LT(UsePtr(shm_addr1), UsePtr(default_as_udp_addr2));
	EXPECT_LT(UsePtr(shm_addr1), UsePtr(shm_addr3));

	EXPECT_NE(UsePtr(simul_addr), UsePtr(udp_addr2));
	EXPECT_NE(UsePtr(udp_addr1), UsePtr(auto_as_udp_addr2));
	EXPECT_NE(UsePtr(tcp_as_udp_addr2), UsePtr(udp_addr2));
	EXPECT_NE(UsePtr(default_port_as_udp_addr2), UsePtr(default_as_udp_addr2));
	EXPECT_NE(UsePtr(shm_addr1), UsePtr(default_as_udp_addr2));
	EXPECT_NE(UsePtr(shm_addr1), UsePtr(shm_addr3));
}

TEST(cable_Test, selector_equall) {
	auto udp_addr1 = c_cable_base_addr::cable_make_addr("udp:192.166.218.58:9042");
	c_card_selector selA( std::move( udp_addr1 ));
	EXPECT_FALSE( selA < selA );
	EXPECT_FALSE( selA != selA );
	EXPECT_TRUE( selA == selA );
}

TEST(c_asioservice_manager_test, resize_to_at_least_test)
{
	g_dbg_level_set(170, "reduce marks and warnings spam from tests (int_to_enum etc)");
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
	EXPECT_THROW(manager.resize_to_at_least(manager.capacity()+1), err_check_input);
}

TEST(c_assioservice_manager_test, get_next_ioservice )
{
	g_dbg_level_set(170, "reduce marks and warnings spam from tests (int_to_enum etc)");
	EXPECT_THROW(c_asioservice_manager(0), err_check_prog);
	c_asioservice_manager man(1);
	size_t size = man.capacity();

	g_dbg_level_set(160,"disable marks from get_next_ioservice");

	do
	{
		std::map<const boost::asio::io_service *, size_t> services;
		c_asioservice_manager manager(size);
		for (size_t i=0; i < size; i++)
		{
			services[&manager.get_next_ioservice()] = 0;
		}
		size_t iter = 5;
		for (size_t i=0; i < size*iter; i++)
		{
			++(services.at(&manager.get_next_ioservice()));
		}
		for (size_t i=0; i < size; i++)
		{
			EXPECT_GT(services.at(&manager.get_next_ioservice()), iter - 2) ;
		}
		size /= 2;
	}while (size >= 1);
}

