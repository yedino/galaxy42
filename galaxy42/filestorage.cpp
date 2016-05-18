#include "filestorage.hpp"


bool filestorage::is_file_ok(const std::string &filename) {
	using namespace boost::filesystem;

	path p(filename);

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

	} catch (const filesystem_error& ex) {
		std::cout << ex.what() << std::endl;
		return 0;
	}
	return 1;
}

bool filestorage::create_parent_dir(const std::string &filename) {

	boost::filesystem::path file(filename);
	boost::filesystem::path parent_path = file.parent_path();

	// if exist
	if(!boost::filesystem::exists(parent_path)) {
		bool success = boost::filesystem::create_directories(parent_path);
		if(!success) {
			throw std::invalid_argument("fail to create not existing directory"
										+ std::string(parent_path.c_str()));
		}
		return 1;
	}
	return 0;
}
