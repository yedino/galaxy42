#include "gtest/gtest.h"
#include "../filestorage.hpp"

TEST(filestorage, custom_string_save) {

	std::cout << "filestorage tests" << std::endl;
}

TEST(filestorage, create_path) {

	std::string home(getenv("HOME"));

	boost::filesystem::path PRV_dir = filestorage::create_path_for(e_filestore_wallet_galaxy_ipkeys_PRV);
	std::cout << PRV_dir << std::endl;

	std::string PRV_full = filestorage::prepare_file_for_write(e_filestore_wallet_galaxy_ipkeys_PRV, "key");
	std::cout << PRV_full << std::endl;

	boost::filesystem::path pub_dir = filestorage::create_path_for(e_filestore_galaxy_ipkeys_pub);
	std::cout << pub_dir << std::endl;

	std::string pub_full = filestorage::prepare_file_for_write(e_filestore_galaxy_ipkeys_pub, "key");
	std::cout << pub_full << std::endl;
}
