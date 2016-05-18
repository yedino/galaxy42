#ifndef C_FILESYSTEM_HPP
#define C_FILESYSTEM_HPP
#include "libs0.hpp"
#include <boost/filesystem.hpp>
#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

enum t_filestore : unsigned char {
	e_filestore_wallet_galaxy_ipkeys_PRV = 0,
	e_filestore_wallet_galaxy_ipkeys_pub = 1
};

class filestorage {
public:
	filestorage() = delete;


	static void save_string(t_filestore file_type,
					 const std::string &filename,
					 const std::string &data);

	static void save_string_mlocked(t_filestore file_type,
							 const std::string &filename,
							 const sodiumpp::locked_string &locked_data);

	/**
	 * check if file is a regural file and can be correctly loaded
	 */
	static bool is_file_ok(const std::string &filename);

	static std::string create_path_for(t_filestore file_type) {

		boost::filesystem::path user_home(getenv("HOME"));
		std::cout << user_home << std::endl;
		return "";
	}

	static std::string prepare_file_for_write(t_filestore file_type, const std::string &filename) {
		switch (file_type) {
			case e_filestore_wallet_galaxy_ipkeys_pub: { 	//...keyid .... filename = getenv("USER").".config/antinet......" + name + ".PRIVATE" ;
				break;	//mod=0700 for private key
			}
			case e_filestore_wallet_galaxy_ipkeys_PRV: {
			//mod=0755 for public
			}
		}

	//mkdir_tree( .... WITHOUT FILE NAME ;) );
	//chmod - dir
	//test if dir exists ...

	//create empty file
	//chmod on file
	//test if file exist ...
	//return filename;
	}


	/**
	 * creating directory for given filename path
	 * return 1 if directory was created
	 * return 0 if directory alredy exist
	 */
	static bool create_parent_dir(const std::string &filename);
};

#endif // C_FILESYSTEM_HPP
