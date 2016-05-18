#include "filestorage.hpp"


bool filestorage::is_file_ok(const std::string &filename) {

	fs::path p(filename);

	try {
		if (exists(p)) {    // does p actually exist?
			if (is_regular_file(p)) {       // is p a regular file?
				// std::cout << p << " size is " << file_size(p) << std::endl;  //dbg

			} else if (is_directory(p)) {     // is p a directory?
				std::cout << p << " is a directory" << std::endl;
				return 0;

			} else {
				std::cout << p << " exists, but is neither a regular file nor a directory" << std::endl;
				return 0;
			}
		} else {
			std::cout << p << " does not exist" << std::endl;
			return 0;
		}

	} catch (const fs::filesystem_error& ex) {
		std::cout << ex.what() << std::endl;
		return 0;
	}
	return 1;
}

boost::filesystem::path filestorage::create_path_for(t_filestore file_type) {

	fs::path user_home(getenv("HOME"));
	fs::path full_path(user_home.c_str());
	switch(file_type) {
	case e_filestore_wallet_galaxy_ipkeys_PRV: {
		full_path += "/.config/antinet/galaxy42/wallet/";
		break;
	}
	case e_filestore_galaxy_ipkeys_pub: {
		full_path += "/.config/antinet/galaxy42/";
		break;
	}
	}
	// std::cout << user_home << std::endl; //dbg
	// std::cout << full_path << std::endl; //dbg
	return full_path;
}

std::string filestorage::prepare_file_for_write(t_filestore file_type, const std::string &filename) {

	// creating directory tree if necessary
	fs::path full_path = create_path_for(file_type);

	// connect parent path with filename
	fs::path file_with_path(full_path); file_with_path += filename;
	switch (file_type) {
	case e_filestore_galaxy_ipkeys_pub: {
		file_with_path += ".public";
		//mod=0700 for private key
		break;
	}
	case e_filestore_wallet_galaxy_ipkeys_PRV: {
		file_with_path += ".PRIVATE";
		//mod=0755 for public
		break;
	}
	}

	// std::cout << file_with_path << std::endl; //dbg

	// In code below we want to create an empty file which will help us to open and write down it without any errors
	/* dont work
	boost::filesystem::ofstream empty_file;
	empty_file.open(file_with_path);
	if(!is_file_ok(file_with_path.c_str())) {
		throw std::invalid_argument(__func__ + std::string(": fail to create empty file on given path and name"));
	}
	*/
	return std::string(file_with_path.c_str());
}

bool filestorage::create_parent_dir(const std::string &filename) {

	fs::path file(filename);
	fs::path parent_path = file.parent_path();

	// if exist
	if(!fs::exists(parent_path)) {
		bool success = fs::create_directories(parent_path);
		if(!success) {
			throw std::invalid_argument("fail to create not existing directory"
										+ std::string(parent_path.c_str()));
		}
		return 1;
	}
	return 0;
}
