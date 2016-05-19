#include "gtest/gtest.h"
#include "../filestorage.hpp"

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

TEST(filestorage, custom_string_save) {

	std::cout << "filestorage tests" << std::endl;
}

TEST(filestorage, create_path) {

	std::string home(getenv("HOME"));
	std::cout << "Home dir: "  << home << std::endl;

	fs::path PRV_dir = filestorage::create_path_for(e_filestore_wallet_galaxy_ipkeys_PRV, "key");
	std::cout << PRV_dir << std::endl;

	fs::path PRV_full = filestorage::prepare_file_for_write(e_filestore_wallet_galaxy_ipkeys_PRV, "key");
	std::cout << PRV_full << std::endl;

	std::cout << "Removing PRV: " << filestorage::remove(PRV_full.native()) << std::endl;

	fs::path pub_dir = filestorage::create_path_for(e_filestore_galaxy_ipkeys_pub, "key");
	std::cout << pub_dir << std::endl;

	fs::path pub_full = filestorage::prepare_file_for_write(e_filestore_galaxy_ipkeys_pub, "key");
	std::cout << pub_full << std::endl;

	std::cout << "Removing pub: " << filestorage::remove(pub_full.native()) << std::endl;

	//
	// std::string
	//

	// prepare files
	fs::path pub_full01 = filestorage::prepare_file_for_write(e_filestore_galaxy_ipkeys_pub, "key01");
	fs::path pub_full02 = filestorage::prepare_file_for_write(e_filestore_galaxy_ipkeys_pub, "key02");

	// saving data
	filestorage::save_string(e_filestore_galaxy_ipkeys_pub, "key01", "public data01");
	filestorage::save_string(e_filestore_galaxy_ipkeys_pub, "key02", "public data02");

	// load data
	std::cout << "Loaded from first pub: "
			  << filestorage::load_string(e_filestore_galaxy_ipkeys_pub, "key01") << std::endl;
	std::cout << "Loaded from second pub: "
			  << filestorage::load_string(e_filestore_galaxy_ipkeys_pub, "key02") << std::endl;

	// cleaning
	std::cout << "Removing key01.public: " << filestorage::remove(pub_full01.native()) << std::endl;
	std::cout << "Removing key02.public: " << filestorage::remove(pub_full02.native()) << std::endl;

	//
	// sodiumpp::locked_string
	//

	// prepare files
	fs::path PRV_full01 = filestorage::prepare_file_for_write(e_filestore_wallet_galaxy_ipkeys_PRV, "key01");
	fs::path PRV_full02 = filestorage::prepare_file_for_write(e_filestore_wallet_galaxy_ipkeys_PRV, "key02");

	// prepare memory locked strings  UNSAFE FOR TEST
	sodiumpp::locked_string l_str01 = sodiumpp::locked_string::unsafe_create("private data01");
	sodiumpp::locked_string l_str02 = sodiumpp::locked_string::unsafe_create("private data02");

	// saving data
	filestorage::save_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, "key01", l_str01);
	filestorage::save_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, "key02", l_str02);

	// load data
	std::cout << "Loaded from PRV: "
			  << filestorage::load_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, "key01").c_str() << std::endl;
	std::cout << "Loaded from PRV: "
			  << filestorage::load_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, "key02").c_str() << std::endl;

	// cleaning
	std::cout << "Removing key01.PRIVATE: " << filestorage::remove(PRV_full01.native()) << std::endl;
	std::cout << "Removing key02.PRIVATE: " << filestorage::remove(PRV_full02.native()) << std::endl;
}
