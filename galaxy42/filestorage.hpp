#ifndef C_FILESYSTEM_HPP
#define C_FILESYSTEM_HPP

#include "libs0.hpp"
#include "gtest/gtest_prod.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

namespace fs=boost::filesystem;

enum t_filestore : unsigned char {
	e_filestore_galaxy_wallet_PRV = 0,
	e_filestore_galaxy_pub = 1,
	e_filestore_galaxy_sig = 2,

	e_filestore_local_path = 3
};
// TODO doc
// TODO more tests
class overwrite_error: public std::runtime_error {
public:
	explicit overwrite_error(const std::string &msg);
};



class filestorage {
public:
	filestorage() = delete;

	static void save_string(t_filestore file_type,
							const std::string &filename,
							const std::string &data,
							bool overwrite = false);

	static void save_string_mlocked(t_filestore file_type,
									const std::string &filename,
									const sodiumpp::locked_string &locked_data,
									bool overwrite = false);

	static std::string load_string(t_filestore file_type,
								   const std::string &filename);

	static sodiumpp::locked_string load_string_mlocked(t_filestore file_type,
													   const std::string &filename);

	/**
	 * @brief check if file can be correctly loaded
	 * @param filename
	 * @return true if file is a regular file
	 */
	static bool is_file_ok(const std::string &filename);

	/**
	 * @brief remove
	 * @param p
	 * @return true if file was successful removed
	 */
	static bool remove(const std::string &p);

private:
	FRIEND_TEST(filestorage, create_path);

	// TODO chmod's
	/**
	 * @brief prepare_file_for_write
	 * @param file_type
	 * @param filename
	 * @param overwrite allow to overwrite file (default = false)
	 * @return full path with fixed filename
	 */
	static fs::path prepare_file_for_write(t_filestore file_type,
										   const std::string &filename,
										   bool overwrite = false);


	static fs::path create_path_for(t_filestore file_type,
									const fs::path &filename);
	/**
	 * @brief creating directory for given filename path
	 * @returns true if directory was created
	 *  		false if directory alredy exist
	 */
	static bool create_parent_dir(const fs::path &filename);

	static fs::path get_path_for(t_filestore file_type,
								 const fs::path &filename);
};

#endif // C_FILESYSTEM_HPP
