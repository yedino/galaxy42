// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef MO_READER_HPP
#define MO_READER_HPP

#include <fstream>
#include <map>
#include <string>

// https://www.gnu.org/software/gettext/manual/html_node/MO-Files.html
class mo_file_reader {
public:
	void read_file();
	/**
	* @param path Path to directory contains /XX/LC_MESSAGES/file.mo where xx is language tag
	*/
	void add_messages_dir(const std::string &path);
	void add_messages_dir(std::string &&path);

	/**
	* @param name Name of mo file
	*/
	void add_mo_filename(const std::string &name);
	void add_mo_filename(std::string &&name);

	std::string gettext(const std::string &original_string);
	// TODO private

private:
	std::map<std::string, std::string> m_translation_map; ///< original string => translation
	uint32_t read_section();
	std::string load_string_with_offset(size_t offset);
	std::string get_system_lang_short_name() const; ///< returns 639-1 code
	std::ifstream m_ifstream;
	std::string m_messages_dir;
	std::string m_mo_filename;
};

#endif // MO_READER_HPP
