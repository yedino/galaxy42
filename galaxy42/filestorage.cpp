#include "filestorage.hpp"
#include "../text_ui.hpp"

overwrite_error::overwrite_error(const std::string &msg) : std::runtime_error(msg)
{ }


void filestorage::save_string(t_filestore file_type,
							  const std::string &filename,
							  const std::string &data,
							  bool overwrite) {

	if (filename == "") {
		throw std::invalid_argument("Fail to open file for write: empty filename");
	}

	fs::path file_with_path;
	try {

		file_with_path = prepare_path_for_write(file_type, filename, overwrite);
		fs::ofstream file(file_with_path, std::ios::out | std::ios::binary);

		if (file.is_open()) {
			file << data;
		} else {
			throw std::invalid_argument("Fail to open file for write: " + filename);
		}
		file.close();
		_dbg2("Successfully saved string to:" << file_with_path.native());

	} catch(overwrite_error &err) {
		std::cout << err.what() << std::endl;
		if(text_ui::ask_user_forpermission("overwrite file?")){
			save_string(file_type, filename, data, true);
		} else {
			_dbg2("Fail to save string");
		}
	}
}

void filestorage::save_string_mlocked(t_filestore file_type,
									  const std::string &filename,
									  const sodiumpp::locked_string &locked_data,
									  bool overwrite) {

	if (filename == "") {
		throw std::invalid_argument("Fail to open file for write: empty filename");
	}

	fs::path file_with_path;
	try {

		file_with_path = prepare_path_for_write(file_type, filename, overwrite);
		FILE *f_ptr;

		f_ptr = std::fopen(file_with_path.c_str(), "w");
		// magic 1 is the size in bytes of each element to be written
		std::fwrite(locked_data.c_str(), 1, locked_data.size(), f_ptr);

		std::fclose(f_ptr);
		_dbg2("Successfully saved mlocked string to:" << file_with_path.native());

	} catch(overwrite_error &err) {
		std::cout << err.what() << std::endl;
		if(text_ui::ask_user_forpermission("overwrite file?")){
			save_string_mlocked(file_type, filename, locked_data, true);
		} else {
			_dbg2("Fail to save mlocked string");
		}
	}
}

std::string filestorage::load_string(t_filestore file_type,
									 const std::string &filename) {
	std::string content;

	fs::path file_with_path = get_full_path(file_type, filename);
	_dbg2("Loading file path: " << file_with_path.native());

	if (!is_file_ok(file_with_path.native())) {
		throw std::invalid_argument("Fail to open file for read: " + filename);
	} else {
		fs::ifstream ifs(file_with_path);
		content.assign( (std::istreambuf_iterator<char>(ifs) ),
						(std::istreambuf_iterator<char>()  ) );

		ifs.close();
	}
	_dbg2("Successfully loaded string from:" << file_with_path.native());
	return content;
}

sodiumpp::locked_string filestorage::load_string_mlocked(t_filestore file_type,
														 const std::string &filename) {
	FILE * f_ptr;
	fs::path file_with_path = get_full_path(file_type, filename);

	f_ptr = std::fopen(file_with_path.c_str(), "r");

	if (f_ptr == NULL){
		throw std::invalid_argument("Fail to open mlocked file for read: " + filename);
	}

	std::fseek(f_ptr, 0L, SEEK_END);
	size_t content_size = static_cast<size_t>(ftell(f_ptr));

	std::rewind(f_ptr);

	sodiumpp::locked_string content(content_size);

	size_t byte_read = std::fread(content.buffer_writable(), 1, content_size, f_ptr);
	if (byte_read != content_size) {
		std::string err_msg = "Fail to read all content of file: " + filename;
		err_msg += " read: [" + std::to_string(byte_read);
		err_msg += "] bytes of [" + std::to_string(content_size) + "]";
		throw std::invalid_argument(err_msg);
	}
	std::fclose(f_ptr);

	_dbg2("Successfully loaded mlocked string from:" << file_with_path.native());
	return content;
}

bool filestorage::is_file_ok(const std::string &filename) {
	fs::path p(filename);
	try {
		if (fs::exists(p)) {    // does p actually exist?
			if (fs::is_regular_file(p)) {       // is p a regular file?
			} else if (fs::is_directory(p)) {     // is p a directory?
				// std::cout << p << " is a directory" << std::endl; // dbg
				return 0;
			} else {
				// std::cout << p << " exists, but is neither a regular file nor a directory" << std::endl; // dbg
				return 0;
			}
		} else {
			// std::cout << p << " does not exist" << std::endl; // dbg
			return 0;
		}
	} catch (const fs::filesystem_error& ex) {
		std::cout << ex.what() << std::endl;
		return 0;
	}
	return 1;
}

bool filestorage::remove(const std::string &p) {
	fs::path path_to_remove(p);
	return fs::remove(path_to_remove);
}

std::vector<std::string> filestorage::get_file_list(const boost::filesystem::path &path) {
	std::vector<std::string> file_list;
	if (!path.empty()) {
		fs::path apk_path(path);
		fs::recursive_directory_iterator end;

		for (fs::recursive_directory_iterator i(apk_path); i != end; ++i) {
			const fs::path cp = (*i);
			std::string filename = extract_filename(cp.native());
			file_list.push_back(filename);
		}
	}
	return file_list;
}

fs::path filestorage::get_full_path(t_filestore file_type,
								   const std::string &filename) {

	fs::path full_path = get_parent_path(file_type, filename);
	full_path += filename;
	switch (file_type) {
		case e_filestore_galaxy_wallet_PRV: {
			full_path += ".PRV";
			break;
		}
		case e_filestore_galaxy_pub: {
			full_path += ".pub";
			break;
		}
		case e_filestore_galaxy_sig: {
			full_path += ".sig";
			break;
		}
	case e_filestore_galaxy_instalation_key_conf: {
			full_path += ".conf";
			break;
		}
		case e_filestore_local_path: {
			break;
		}
	}
	// _dbg3("full_path " << full_path);
	return full_path;
}

fs::path filestorage::get_parent_path(t_filestore file_type,
									const std::string &filename) {

	fs::path user_home(getenv("HOME"));
	fs::path parent_path(user_home.c_str());

	switch (file_type) {
		case e_filestore_galaxy_wallet_PRV: {
			parent_path += "/.config/antinet/galaxy42/wallet/";
			break;
		}
		case e_filestore_galaxy_pub: {
			parent_path += "/.config/antinet/galaxy42/public/";
			break;
		}
		case e_filestore_galaxy_sig: {
			parent_path += "/.config/antinet/galaxy42/public/";
			break;
		}
	case e_filestore_galaxy_instalation_key_conf: {
			parent_path += "/.config/antinet/galaxy42/config/";
			break;
		}
		case e_filestore_local_path: {
			fs::path file_path(filename);
			parent_path = file_path.parent_path();
			break;
		}
	}

	return parent_path;
}

fs::path filestorage::prepare_path_for_write(t_filestore file_type,
											 const std::string &input_name,
											 bool overwrite) {
	fs::path file_with_path;
	try {

		std::string filename = extract_filename(input_name);
		// creating directory tree if necessary
		file_with_path = create_path_for(file_type, filename);

		// prevent overwriting
		if(is_file_ok(file_with_path.native()) &&  !overwrite) {
			std::string err_msg(file_with_path.native()
								+ std::string(": file existing, it can't be overwrite [overwrite=")
								+ std::to_string(overwrite)
								+ std::string("]"));
			throw overwrite_error(err_msg);
		}

		// In code below we want to create an empty file which will help us to open and write down it without any errors
		boost::filesystem::ofstream empty_file;
		empty_file.open(file_with_path);
		empty_file.close();
		if (!is_file_ok(file_with_path.c_str())) {
			std::string err_msg(__func__ + std::string(": fail to create empty file on given path and name"));
			throw std::invalid_argument(err_msg);
		}

		// TODO perrmisions
		switch (file_type) {
			case e_filestore_galaxy_pub: {
				//mod=0700 for private key
				break;
			}
			case e_filestore_galaxy_wallet_PRV: {
				//mod=0755 for public
				break;
			}
			case e_filestore_galaxy_sig: {
				break;
			}
		case e_filestore_galaxy_instalation_key_conf: {
				break;
			}
			case e_filestore_local_path: {
				break;
			}
		}

	} catch (fs::filesystem_error & err) {
		std::cout << err.what() << std::endl;
	}
	return file_with_path;
}

fs::path filestorage::create_path_for(t_filestore file_type,
									  const std::string &filename) {

	// connect parent path with filename
	fs::path full_path = get_full_path(file_type, filename);
	std::cout << "full_path: " << full_path.native() << std::endl;
	create_parent_dir(full_path);
	return full_path;
}

std::string filestorage::extract_filename(const std::string &string_path) {
	fs::path try_path(string_path);
	return try_path.filename().native();
}

bool filestorage::create_parent_dir(const fs::path &file_path) {

	fs::path file(file_path);
	fs::path parent_path = file.parent_path();

	if (parent_path.empty()) {
		return 1;
	}

	// if exist
	if (!fs::exists(parent_path)) {
		bool success = fs::create_directories(parent_path);
		if (!success) {
			throw std::invalid_argument("fail to create not existing directory"
										+ std::string(parent_path.c_str()));
		}
		return 1;
	}
	return 0;
}
