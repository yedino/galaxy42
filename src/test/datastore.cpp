// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../datastore.hpp"

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

TEST(datastore, custom_string_save) {

	_dbg2("datastore tests");
}

TEST(datastore, prepare_path) {

	std::string home(getenv("HOME"));
	_dbg2("Home dir: "  << home);

	b_fs::path PRV_dir = datastore::create_path_for(e_datastore_galaxy_wallet_PRV, "bannedname!_key");
	_dbg2(PRV_dir);

	b_fs::path PRV_full = datastore::prepare_path_for_write(e_datastore_galaxy_wallet_PRV, "bannedname!_key", true);
	_dbg2(PRV_full);

	datastore::save_string(e_datastore_galaxy_wallet_PRV, "bannedname!_key", "priv data01", true);
	// try to overvrite path for PRV key
	EXPECT_THROW({
		b_fs::path PRV_full = datastore::prepare_path_for_write(e_datastore_galaxy_wallet_PRV, "bannedname!_key");
	}, overwrite_error);

	// removing
	ASSERT_TRUE(datastore::remove(PRV_full.string()));

	b_fs::path pub_dir = datastore::create_path_for(e_datastore_galaxy_pub, "bannedname!_key");
	_dbg2(pub_dir);

	b_fs::path pub_full = datastore::prepare_path_for_write(e_datastore_galaxy_pub, "bannedname!_key", true);
	_dbg2(pub_full);

	// removing
	ASSERT_TRUE(datastore::remove(pub_full.string()));

}

TEST(datastore, save_empty) {

	std::string empty_filename;
	// save string
	EXPECT_THROW({
		datastore::save_string(e_datastore_galaxy_pub, empty_filename, "data");
	}, std::invalid_argument);

	// save mlocked string
/*
	sodiumpp::locked_string l_data = sodiumpp::locked_string::unsafe_create("private data");
	EXPECT_THROW({
		datastore::save_string_mlocked(e_datastore_galaxy_pub, empty_filename, l_data);
	}, std::invalid_argument);
*/
}

TEST(datastore, load_nonexistent) {

	std::string non_existent_file = "test_file_that_should_not_exist!.test";
	// string
	EXPECT_THROW({
		datastore::load_string(e_datastore_galaxy_instalation_key_conf, non_existent_file);
	},  expected_not_found);
	// strin mlocked
	EXPECT_THROW({
		datastore::load_string_mlocked(e_datastore_galaxy_instalation_key_conf, non_existent_file);
	},  expected_not_found);
}

TEST(datastore, bad_file_confdir) {

	std::string test_conf_name = "test_non_important_conffile.configuration";
	b_fs::path parent_dirname  = datastore::get_parent_path(e_datastore_galaxy_instalation_key_conf, test_conf_name);

	// trying to open directory
	ASSERT_FALSE(datastore::is_file_ok(parent_dirname.string()));
}

TEST(datastore, file_list_sig) {

	//  adding some signatures for this test
	std::string sig1 = "bannedname!_test_sig01";
	std::string sig2 = "bannedname!_test_sig02";

	datastore::save_string(e_datastore_galaxy_sig, sig1, "my_invalid_sig", true);
	datastore::save_string(e_datastore_galaxy_sig, sig2, "my_invalid_sig", true);

	auto sig_path = datastore::get_parent_path(e_datastore_galaxy_sig,"");
	auto signatures = datastore::get_file_list(sig_path);

	_dbg1("TEST file list:");
	for(auto &a : signatures) {
		_dbg1(a);
	}
	// cleaning
	ASSERT_TRUE(datastore::remove(e_datastore_galaxy_sig, sig1));
	ASSERT_TRUE(datastore::remove(e_datastore_galaxy_sig, sig2));
}

TEST(datastore, write_load_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string
	b_fs::path pub_full01 = datastore::prepare_path_for_write(e_datastore_galaxy_pub, "bannedname!_key01", true);
	b_fs::path pub_full02 = datastore::prepare_path_for_write(e_datastore_galaxy_pub, "bannedname!_key02", true);

	// saving data
	datastore::save_string(e_datastore_galaxy_pub, "bannedname!_key01", "public data01", true);
	datastore::save_string(e_datastore_galaxy_pub, "bannedname!_key02", "public data02", true);

	// load data
	datastore::load_string(e_datastore_galaxy_pub, "bannedname!_key01");
	datastore::load_string(e_datastore_galaxy_pub, "bannedname!_key02");

	// cleaning
	ASSERT_TRUE(datastore::remove(pub_full01.string()));
	ASSERT_TRUE(datastore::remove(pub_full02.string()));
}
/*
TEST(datastore, write_load_mlocked_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string_mlocked
	b_fs::path PRV_full01 = datastore::prepare_path_for_write(e_datastore_galaxy_wallet_PRV, "bannedname!_key01", true);
	b_fs::path PRV_full02 = datastore::prepare_path_for_write(e_datastore_galaxy_wallet_PRV, "bannedname!_key02", true);

	// prepare memory locked strings  UNSAFE FOR TEST
	sodiumpp::locked_string l_str01 = sodiumpp::locked_string::unsafe_create("private data01");
	sodiumpp::locked_string l_str02 = sodiumpp::locked_string::unsafe_create("private data02");

	// saving data
	datastore::save_string_mlocked(e_datastore_galaxy_wallet_PRV, "bannedname!_key01", l_str01, true);
	datastore::save_string_mlocked(e_datastore_galaxy_wallet_PRV, "bannedname!_key02", l_str02, true);

	// load data
	datastore::load_string_mlocked(e_datastore_galaxy_wallet_PRV, "bannedname!_key01");
	datastore::load_string_mlocked(e_datastore_galaxy_wallet_PRV, "bannedname!_key02");

	// cleaning
	ASSERT_TRUE(datastore::remove(PRV_full01.string()));
	ASSERT_TRUE(datastore::remove(PRV_full02.string()));
}
*/
