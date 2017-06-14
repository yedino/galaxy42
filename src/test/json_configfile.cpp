#include <gtest/gtest.h>
#include <c_json_load.hpp>
#include <c_json_genconf.hpp>

#include <iostream>
#include <datastore.hpp>

TEST(json_configfile, defaul_generate_and_load) {

	std::string conf_filename = "galaxy.conf";
	EXPECT_NO_THROW( {
		c_json_genconf::genconf();
		c_galaxyconf_load galaxyconf(conf_filename);
	}
	);

	// delete test files
	datastore::remove(conf_filename);
	datastore::remove("connect_from.my.conf");
	datastore::remove("connect_to.my.conf");
	datastore::remove("connect_to.seed.conf");

}
