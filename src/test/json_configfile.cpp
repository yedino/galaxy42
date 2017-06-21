#include <gtest/gtest.h>
#include <c_json_load.hpp>
#include <c_json_genconf.hpp>

#include <iostream>
#include <datastore.hpp>

void generate_conf_files() {
	// default main conffile galaxy42.conf
	c_json_genconf::genconf();
}

void delete_conf_files() {
	// delete test files
	datastore::remove("galaxy.conf");
	datastore::remove("connect_from.my.conf");
	datastore::remove("connect_to.my.conf");
	datastore::remove("connect_to.seed.conf");
}

TEST(json_configfile, load_keys) {

	generate_conf_files();
	c_galaxyconf_load galaxyconf;
	auto keypair_struct = galaxyconf.my_keypair_load();
	EXPECT_EQ(keypair_struct.m_private_key_type,"master-dh");
	EXPECT_EQ(keypair_struct.m_private_key,"ed25519:1fffffffffffffffffffaaaaaaaaaaabbbbbbbbbbbbbbbbcccccccccccc11111");
	EXPECT_EQ(keypair_struct.m_public_key,"ed25519:1111111112222222233333333344444444445555555666666666.k");
	EXPECT_EQ(keypair_struct.m_ipv6,"fd42:aaaa:bbbb:cccc:aaaa:bbbb:cccc:dddd");

	delete_conf_files();
}

TEST(json_configfile, peer_references_haship) {

	generate_conf_files();
	c_galaxyconf_load galaxyconf;
	auto peer_references = galaxyconf.get_peer_references();

	std::string haship01 = peer_references.at(0).haship_addr.get_hip_as_string(true);
	std::string haship02 = peer_references.at(1).haship_addr.get_hip_as_string(true);
	std::string haship03 = peer_references.at(2).haship_addr.get_hip_as_string(true);
	std::string haship04 = peer_references.at(3).haship_addr.get_hip_as_string(true);

	// Files "connect_to.my.conf" and "connect_to.seed.conf" are identical
	EXPECT_EQ(haship01,haship03);
	EXPECT_EQ(haship02,haship04);

	EXPECT_EQ(haship01,"fd1c:b8e7:644d:ab7c:a659:4332:47bc:67c4");
	EXPECT_EQ(haship02,"fd1d:a8e7:644d:ab7c:a659:3372:47bc:67c4");

	delete_conf_files();
}

TEST(json_configfile, peer_references_peerip) {

	generate_conf_files();
	c_galaxyconf_load galaxyconf;
	auto peer_references = galaxyconf.get_peer_references();

	// default galaxy42 port
	auto peering_ip01 = c_ip46_addr("192.168.0.57");
	// custom port
	auto peering_ip02 = c_ip46_addr("192.168.0.66",19420);

	EXPECT_EQ(peer_references.at(0).peering_addr, peering_ip01);
	EXPECT_EQ(peer_references.at(1).peering_addr, peering_ip02);

	// Files "connect_to.my.conf" and "connect_to.seed.conf" are identical
	EXPECT_EQ(peer_references.at(2).peering_addr, peering_ip01);
	EXPECT_EQ(peer_references.at(3).peering_addr, peering_ip02);

	delete_conf_files();
}

TEST(json_configfile, auth_password_load) {

	generate_conf_files();
	c_galaxyconf_load galaxyconf;
	auto auth_passwords = galaxyconf.get_auth_passwords();
	EXPECT_EQ(auth_passwords.at(0).m_myname,"default_public_password");
	EXPECT_EQ(auth_passwords.at(0).m_password,"C3yhZ8PjPoVFYwHfw0oKtNLxgMo5V9YUr7r4UXul");

	delete_conf_files();
}
