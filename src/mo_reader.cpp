// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "mo_reader.hpp"
#include <cstring>

#include "libs0.hpp" // for debug

std::atomic<bool> mo_file_reader::s_translation_map_ready(false);
std::map<std::string, std::string> mo_file_reader::m_translation_map;

#ifdef _WIN32
	#include <windows.h>
	#include <winnls.h>
#else
	#include <cstdlib> // getenv
#endif

void mo_file_reader::read_file() {
	std::string locale_lang_short_name =get_system_lang_short_name();
	std::cerr << "Reading translations for locale short=[" << locale_lang_short_name << "]" << std::endl;
	const std::string filename = m_messages_dir + "/" + locale_lang_short_name + "/LC_MESSAGES/" + m_mo_filename;
	m_ifstream.open(filename, std::ios::in | std::ios::binary);
	if (!m_ifstream.is_open()) throw std::runtime_error("mo file open error, filename: " + filename);

	uint32_t magic_number = read_section();
	if (magic_number == 0xde120495) {
		throw std::runtime_error("mo files with this endianness is not supported (we use little-endian)");
	}
	if ((magic_number != 0x950412de)) {
		throw std::runtime_error("mo file bad format (wrong magic number)");
	}

	uint32_t file_format_revision = read_section();
	uint32_t number_of_strings = read_section();
	if (number_of_strings == 0)
		throw std::runtime_error("Not found translation strings");

	uint32_t offset_of_table_with_original_strings = read_section();
	uint32_t offset_of_table_with_translation_strings = read_section();
	uint32_t size_of_hashing_table = read_section();
	uint32_t offset_of_hashing_table = read_section();

	pfp_dbg1("MO format: " <<  file_format_revision << ", " << size_of_hashing_table << "@" << offset_of_hashing_table );

	// loadnig to map
	for (uint32_t i = 0; i < number_of_strings; i++) {
		//i = 17;
		m_ifstream.seekg(offset_of_table_with_original_strings + i * 8);
		uint32_t length = read_section();
		uint32_t offset = read_section();

		m_ifstream.seekg(offset);
		std::string orig_str(length, 0);
		m_ifstream.read(&orig_str[0], length);

		m_ifstream.seekg(offset_of_table_with_translation_strings + i * 8);
		length = read_section();
		offset = read_section();
		m_ifstream.seekg(offset);

		std::string translate_str(length, 0);
		m_ifstream.read(&translate_str[0], length);

		m_translation_map.emplace(std::move(orig_str), std::move(translate_str));
	}
	s_translation_map_ready = true;
}

void mo_file_reader::add_messages_dir(const std::string &path) {
	m_messages_dir = path;
	pfp_fact("Starting translations reader (mo_reader) for path: [" << m_messages_dir << "]");
}

void mo_file_reader::add_messages_dir(std::string &&path) {
	m_messages_dir = std::move(path);
}

std::string mo_file_reader::gettext(const std::string &original_string) {
	if (!s_translation_map_ready) return original_string;
	try {
		return m_translation_map.at(original_string);
	}
	catch (const std::out_of_range &e) {
		return original_string;
	}
}

/// This function reads 4-byte integer, in little-endian encoding. (this is encoding we use in Yedino/Galaxy42 project)
uint32_t mo_file_reader::read_section() {
	std::array<unsigned char, 4> buffer;
	m_ifstream.read(reinterpret_cast<char *>(&buffer[0]), buffer.size());
	uint32_t ret = 0;
	static_assert(sizeof(ret) == sizeof(char) * 4, "");
	ret = (buffer[0]<<0) + (buffer[1]<<8) + (buffer[2]<<16) + (buffer[3]<<24);
	return ret;
}

std::string mo_file_reader::get_system_lang_short_name() const {
#ifndef _WIN32
	char *env = std::getenv("LANG");
	std::string lang_env;
	if (env != nullptr) {
		lang_env = std::getenv("LANG");
	}
	if (lang_env.size() >= 2 && std::islower(lang_env.at(0)) && std::islower(lang_env.at(1))) {
		return lang_env.substr(0, 2); // return first 2 characters (i.e. "pl" for "pl_PL.UTF-8")
	}
	std::string locale = setlocale(LC_CTYPE, NULL); // just query the locale, without changing it
	bool locale_C_subtype = false; // locale name like "C.UTF-8"
	if (locale.size() >= 2) {
		if ( (locale.at(0)=='C') && (locale.at(1) == '.') ) locale_C_subtype=true;
	}
	if ( (locale == "C") || locale_C_subtype) locale = "en"; // default language
	return locale;
#else
	char lang[3]; // ISO 639-1 == two-letter codes
	int ret = GetLocaleInfoA(
		LOCALE_USER_DEFAULT,
		LOCALE_SISO639LANGNAME,
		lang,
		sizeof(lang)
		);
	if (ret == 0) throw std::runtime_error("GetLocaleInfo error, last error code " + std::to_string(GetLastError()));
	return std::string(lang);
#endif
}

