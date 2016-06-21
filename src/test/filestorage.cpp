// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../filestorage.hpp"

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

TEST(filestorage, custom_string_save) {

	_dbg2("filestorage tests");
}

TEST(filestorage, prepare_path) {

	std::string home(getenv("HOME"));
	_dbg2("Home dir: "  << home);

	b_fs::path PRV_dir = filestorage::create_path_for(e_filestore_galaxy_wallet_PRV, "bannedname!_key");
	_dbg2(PRV_dir);

	b_fs::path PRV_full = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "bannedname!_key", true);
	_dbg2(PRV_full);

	// try to overvrite path for PRV key
	EXPECT_THROW({
		b_fs::path PRV_full = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "bannedname!_key");
	},overwrite_error);

	// removing
	ASSERT_TRUE(filestorage::remove(PRV_full.native()));

	b_fs::path pub_dir = filestorage::create_path_for(e_filestore_galaxy_pub, "bannedname!_key");
	_dbg2(pub_dir);

	b_fs::path pub_full = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "bannedname!_key", true);
	_dbg2(pub_full);

	// removing
	ASSERT_TRUE(filestorage::remove(pub_full.native()));

}

TEST(filestorage, save_empty) {

	std::string empty_filename;
	// save string
	EXPECT_THROW({
		filestorage::save_string(e_filestore_galaxy_pub, empty_filename, "data");
	}, std::invalid_argument);

	// save mlocked string
	sodiumpp::locked_string l_data = sodiumpp::locked_string::unsafe_create("private data");
	EXPECT_THROW({
		filestorage::save_string_mlocked(e_filestore_galaxy_pub, empty_filename, l_data);
	}, std::invalid_argument);

}

TEST(filestorage, load_nonexistent) {

	std::string non_existent_file = "test_file_that_should_not_exist!.test";
	// string
	EXPECT_THROW({
		filestorage::load_string(e_filestore_galaxy_instalation_key_conf, non_existent_file);
	}, std::invalid_argument);
	// strin mlocked
	EXPECT_THROW({
		filestorage::load_string_mlocked(e_filestore_galaxy_instalation_key_conf, non_existent_file);
	}, std::invalid_argument);
}

TEST(filestorage, bad_file_confdir) {

	std::string test_conf_name = "test_non_important_conffile.configuration";
	b_fs::path parent_dirname  = filestorage::get_parent_path(e_filestore_galaxy_instalation_key_conf, test_conf_name);

	// trying to open directory
	ASSERT_FALSE(filestorage::is_file_ok(parent_dirname.native()));
}

TEST(filestorage, file_list_sig) {

	//  adding some signatures for this test
	std::string sig1 = "bannedname!_test_sig01";
	std::string sig2 = "bannedname!_test_sig02";

	filestorage::save_string(e_filestore_galaxy_sig, sig1, "my_invalid_sig", true);
	filestorage::save_string(e_filestore_galaxy_sig, sig2, "my_invalid_sig", true);

	auto sig_path = filestorage::get_parent_path(e_filestore_galaxy_sig,"");
	auto signatures = filestorage::get_file_list(sig_path);

	_dbg1("TEST file list:");
	for(auto &a : signatures) {
		_dbg1(a);
	}
	// cleaning
	ASSERT_TRUE(filestorage::remove(e_filestore_galaxy_sig, sig1));
	ASSERT_TRUE(filestorage::remove(e_filestore_galaxy_sig, sig2));
}

TEST(filestorage, write_load_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string
	b_fs::path pub_full01 = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "bannedname!_key01", true);
	b_fs::path pub_full02 = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "bannedname!_key02", true);

	// saving data
	filestorage::save_string(e_filestore_galaxy_pub, "bannedname!_key01", "public data01", true);
	filestorage::save_string(e_filestore_galaxy_pub, "bannedname!_key02", "public data02", true);

	// load data
	filestorage::load_string(e_filestore_galaxy_pub, "bannedname!_key01");
	filestorage::load_string(e_filestore_galaxy_pub, "bannedname!_key02");

	// cleaning
	ASSERT_TRUE(filestorage::remove(pub_full01.native()));
	ASSERT_TRUE(filestorage::remove(pub_full02.native()));
}

TEST(filestorage, write_load_mlocked_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string_mlocked
	b_fs::path PRV_full01 = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "bannedname!_key01", true);
	b_fs::path PRV_full02 = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "bannedname!_key02", true);

	// prepare memory locked strings  UNSAFE FOR TEST
	sodiumpp::locked_string l_str01 = sodiumpp::locked_string::unsafe_create("private data01");
	sodiumpp::locked_string l_str02 = sodiumpp::locked_string::unsafe_create("private data02");

	// saving data
	filestorage::save_string_mlocked(e_filestore_galaxy_wallet_PRV, "bannedname!_key01", l_str01, true);
	filestorage::save_string_mlocked(e_filestore_galaxy_wallet_PRV, "bannedname!_key02", l_str02, true);

	// load data
	filestorage::load_string_mlocked(e_filestore_galaxy_wallet_PRV, "bannedname!_key01");
	filestorage::load_string_mlocked(e_filestore_galaxy_wallet_PRV, "bannedname!_key02");

	// cleaning
	ASSERT_TRUE(filestorage::remove(PRV_full01.native()));
	ASSERT_TRUE(filestorage::remove(PRV_full02.native()));
}
