// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef MO_READER_HPP
#define MO_READER_HPP

#include <array>
#include <atomic>
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
	* @param name Name of mo file(std::string)
	*/
	template <class T>
	void add_mo_filename(T &&name);

	static std::string gettext(const std::string &original_string);

private:
	static std::map<std::string, std::string> m_translation_map; ///< original string => translation
	uint32_t read_section();
	std::string load_string_with_offset(size_t offset);
	std::string get_system_lang_short_name() const; ///< returns 639-1 code
	std::ifstream m_ifstream;
	std::string m_messages_dir;
	std::string m_mo_filename;
	static std::atomic<bool> s_translation_map_ready;
};


template <class T>
void mo_file_reader::add_mo_filename(T &&name) {
	static_assert(std::is_same<T, const std::string &>::value || std::is_same<T, std::string>::value, "name must be std::string");
	if (!m_mo_filename.empty()) throw std::runtime_error("mo file is open");
	if (name.size() < 4) throw std::invalid_argument("file name is too short");
	m_mo_filename = std::forward<T>(name);
	if (m_mo_filename.substr(m_mo_filename.size() - 3) != ".mo")
		m_mo_filename += ".mo";
}

#endif // MO_READER_HPP
