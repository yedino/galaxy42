// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef C_FILESYSTEM_HPP
#define C_FILESYSTEM_HPP

#include "libs0.hpp"
#include "gtest/gtest_prod.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <sodium.h>
#include <sodiumpp/sodiumpp.h>

namespace b_fs=boost::filesystem;

enum t_datastore : unsigned char {
	e_datastore_galaxy_wallet_PRV = 0,
	e_datastore_galaxy_pub = 1,
	e_datastore_galaxy_sig = 2,
	e_datastore_galaxy_instalation_key_conf = 3,

	e_datastore_local_path = 4
};
// TODO doc
// TODO more tests
class overwrite_error: public std::runtime_error {
public:
	explicit overwrite_error(const std::string &msg);
};



class datastore {
public:
	datastore() = delete;

	static void save_string(t_datastore file_type,
							const std::string &filename,
							const std::string &data,
							bool overwrite = false);

	static void save_string_mlocked(t_datastore file_type,
									const std::string &filename,
									const sodiumpp::locked_string &locked_data,
									bool overwrite = false);

/// new style load string functions - recommended
/// default custom local path gives intuitive behavior, like std functions
///@{
	static std::string load_string(const std::string &filename,
								   t_datastore file_type = e_datastore_local_path);

	static sodiumpp::locked_string load_string_mlocked(const std::string &filename,
													   t_datastore file_type = e_datastore_local_path);

///@}

/// old style load string functions
///@{
	static std::string load_string(t_datastore file_type,
								   const std::string &filename);

	static sodiumpp::locked_string load_string_mlocked(t_datastore file_type,
													   const std::string &filename);
///@}

	/**
	 * @brief check if file can be correctly loaded
	 * @param filename
	 * @return true if file is a regular file
	 * @{
	 */
	static bool is_file_ok(const std::string &filename);
	static bool is_file_ok(const b_fs::path &path);
	/// @}

	/**
	 * @brief remove
	 * @param p
	 * @return true if file was successful removed
	 */
	static bool remove(const std::string &p);
	/**
	 * @brief remove by file_type
	 * @param file_type
	 * @param p filename without path
	 * @return
	 */
	static bool remove(t_datastore file_type, const std::string &p);
	/**
	 * @brief get_file_list vector of strings with file names
	 * @param path directory where we looking for files
	 * @return
	 */
	static std::vector<std::string> get_file_list(const b_fs::path &path);
	/**
	 * @brief get_full_path
	 * @param file_type
	 * @param filename
	 * @return full path with filename included and extension if defined
	 */
	static b_fs::path get_full_path(t_datastore file_type,
								 const std::string &filename);
	/**
	 * @brief get_parent_path
	 * @param file_type
	 * @param filename could be any string for file_type with concrete path
	 * @return directory where specific file_type should be
	 */
	static b_fs::path get_parent_path(t_datastore file_type,
								 const std::string &filename);


private:
	FRIEND_TEST(datastore, prepare_path);
	FRIEND_TEST(datastore, write_load_string);
	FRIEND_TEST(datastore, write_load_mlocked_string);

	// TODO chmod's
	/**
	 * @brief prepare_file_for_write
	 * @param file_type
	 * @param filename
	 * @param overwrite allow to overwrite file (default = false)
	 * @return full path with fixed filename
	 */
	static b_fs::path prepare_path_for_write(t_datastore file_type,
										   const std::string &filename,
										   bool overwrite = false);

	static b_fs::path create_path_for(t_datastore file_type,
									const std::string &filename);
	/**
	 * @brief creating directory for given filename path
	 * @returns true if directory was created
	 *  		false if directory alredy exist
	 */
	static bool create_parent_dir(const b_fs::path &file_path);

	static std::string extract_filename(const std::string &string_path);
};

#endif // C_FILESYSTEM_HPP
