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

	b_fs::path PRV_dir = filestorage::create_path_for(e_filestore_galaxy_wallet_PRV, "key");
	_dbg2(PRV_dir);

	b_fs::path PRV_full = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "key", true);
	_dbg2(PRV_full);

	// try to overvrite path for PRV key
	EXPECT_THROW({
		b_fs::path PRV_full = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "key");
	},overwrite_error);


	_dbg2("Removing PRV: " << filestorage::remove(PRV_full.native()));

	b_fs::path pub_dir = filestorage::create_path_for(e_filestore_galaxy_pub, "key");
	_dbg2(pub_dir);

	b_fs::path pub_full = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "key", true);
	_dbg2(pub_full);

	_dbg2("Removing pub: " << filestorage::remove(pub_full.native()));

}

TEST(filestorage, write_load_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string
	b_fs::path pub_full01 = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "key01", true);
	b_fs::path pub_full02 = filestorage::prepare_path_for_write(e_filestore_galaxy_pub, "key02", true);

	// saving data
	filestorage::save_string(e_filestore_galaxy_pub, "key01", "public data01", true);
	filestorage::save_string(e_filestore_galaxy_pub, "key02", "public data02", true);

	// load data
	_dbg2("Loaded from first pub: "
			  << filestorage::load_string(e_filestore_galaxy_pub, "key01"));
	_dbg2("Loaded from second pub: "
			  << filestorage::load_string(e_filestore_galaxy_pub, "key02"));

	// cleaning
	_dbg2("Removing key01.public: " << filestorage::remove(pub_full01.native()));
	_dbg2("Removing key02.public: " << filestorage::remove(pub_full02.native()));
}

TEST(filestorage, write_load_mlocked_string) {
	// prepare files
	// Not necessary preparing files is aslo a part of save_string_mlocked
	b_fs::path PRV_full01 = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "key01", true);
	b_fs::path PRV_full02 = filestorage::prepare_path_for_write(e_filestore_galaxy_wallet_PRV, "key02", true);

	// prepare memory locked strings  UNSAFE FOR TEST
	sodiumpp::locked_string l_str01 = sodiumpp::locked_string::unsafe_create("private data01");
	sodiumpp::locked_string l_str02 = sodiumpp::locked_string::unsafe_create("private data02");

	// saving data
	filestorage::save_string_mlocked(e_filestore_galaxy_wallet_PRV, "key01", l_str01, true);
	filestorage::save_string_mlocked(e_filestore_galaxy_wallet_PRV, "key02", l_str02, true);

	// load data
	_dbg2("Loaded from PRV: "
			  << filestorage::load_string_mlocked(e_filestore_galaxy_wallet_PRV, "key01").c_str());
	_dbg2("Loaded from PRV: "
			  << filestorage::load_string_mlocked(e_filestore_galaxy_wallet_PRV, "key02").c_str());

	// cleaning
	_dbg2("Removing key01.PRIVATE: " << filestorage::remove(PRV_full01.native()));
	_dbg2("Removing key02.PRIVATE: " << filestorage::remove(PRV_full02.native()));
}
