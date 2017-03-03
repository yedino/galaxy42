#include "gtest/gtest.h"
#include <cable/base/cable_base_addr.hpp>
#include <cable/shm/cable_shm_addr.hpp>
#include <cable/simulation/cable_simul_addr.hpp>
#include <cable/udp/cable_udp_addr.hpp>
#include <iostream>

TEST(cable_Test, operators_test) {
	auto world = make_shared<c_world>();
	auto udp_addr = cable_make_addr("udp:192.166.218.58:9042");
	auto udp_addr2 = cable_make_addr("udp:192.166.218.58:9042");
	auto udp_addr3 = cable_make_addr("udp:192.166.219.58:9042");
	c_cable_simul_addr simul_addr( world->generate_simul_cable() );

	EXPECT_EQ(udp_addr, udp_addr2);
	EXPECT_GT(udp_addr3, udp_addr2);
}
