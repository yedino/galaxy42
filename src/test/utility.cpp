// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../tunserver.hpp"

TEST(utility, parse_ip_number) {
	// g_dbg_level_set(20, "start test");

	pfp_info("Starting test");
	auto args = c_tunserver::get_default_early_argm();
	pfp_info("Spawning server");
	c_tunserver myserver(0, 0, args);

	EXPECT_NO_THROW(myserver.add_peer_simplestring("192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"));
	EXPECT_THROW   (myserver.add_peer_simplestring(""), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("kjfahskdfhsh"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("23947934u43y"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("192.168.2.62:9042fd42:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("d42:10a9:4318:509b:80ab:8042:6275:609b-192.168.2.62:9042"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("192.168.2.62-fd42:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("192.168.2.62-:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("562.486.489.426:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("192.168.2.62:9042-xyz1:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
	EXPECT_THROW   (myserver.add_peer_simplestring("192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b 192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"), std::invalid_argument);
}
