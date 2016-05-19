#include "gtest/gtest.h"
#include "../filestorage.hpp"

TEST(filestorage, custom_string_save) {

	std::cout << "filestorage tests" << std::endl;
}

TEST(filestorage, create_path) {

	std::string home(getenv("HOME"));
	std::cout << "Home dir: "  << home << std::endl;

	fs::path PRV_dir = filestorage::create_path_for(e_filestore_wallet_galaxy_ipkeys_PRV);
	std::cout << PRV_dir << std::endl;

	fs::path PRV_full = filestorage::prepare_file_for_write(e_filestore_wallet_galaxy_ipkeys_PRV, "key");
	std::cout << PRV_full << std::endl;

	std::cout << "Removing PRV: " << filestorage::remove(PRV_full.native()) << std::endl;

	fs::path pub_dir = filestorage::create_path_for(e_filestore_galaxy_ipkeys_pub);
	std::cout << pub_dir << std::endl;

	fs::path pub_full = filestorage::prepare_file_for_write(e_filestore_galaxy_ipkeys_pub, "key");
	std::cout << pub_full << std::endl;

	std::cout << "Removing pub: " << filestorage::remove(pub_full.native()) << std::endl;


	// saving data
	filestorage::save_string(e_filestore_wallet_galaxy_ipkeys_PRV, "key", "private data");
	filestorage::save_string(e_filestore_galaxy_ipkeys_pub, "key", "public data");

	// load data
	std::cout << "Loaded from PRV: "
			  << filestorage::load_string(e_filestore_wallet_galaxy_ipkeys_PRV, "key.PRIVATE") << std::endl;
	std::cout << "Loaded from pub: "
			  << filestorage::load_string(e_filestore_galaxy_ipkeys_pub, "key.public") << std::endl;

	// cleaning
	std::cout << "Removing PRV: " << filestorage::remove(PRV_full.native()) << std::endl;
	std::cout << "Removing pub: " << filestorage::remove(pub_full.native()) << std::endl;
}
