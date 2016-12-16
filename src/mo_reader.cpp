// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "mo_reader.hpp"
#include <cstring>

#ifdef _WIN32
	#include <windows.h>
	#include <winnls.h>
#else
	#include <cstdlib> // getenv
#endif

void mo_file_reader::read_file() {
	const std::string filename = m_messages_dir + "/" + get_system_lang_short_name() + "/LC_MESSAGES/" + m_mo_filename;
	m_ifstream.open(filename, std::ios::in | std::ios::binary); // TODO lang name
	if (!m_ifstream.is_open()) throw std::runtime_error("mo file open error, filename: " + filename);
	uint32_t magic_number = read_section();
	if ((magic_number != 0x950412de) && (magic_number != 0xde120495))
		throw std::runtime_error("mo file bad format (wrong magic number)");

	uint32_t file_format_revision = read_section();
	uint32_t number_of_strings = read_section();
	if (number_of_strings == 0)
		throw std::runtime_error("Not found translation strings");

	uint32_t offset_of_table_with_original_strings = read_section();
	uint32_t offset_of_table_with_translation_strings = read_section();
	uint32_t size_of_hashing_table = read_section();
	uint32_t offset_of_hashing_table = read_section();

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
}

void mo_file_reader::add_messages_dir(const std::string &path) {
	m_messages_dir = path;
}

void mo_file_reader::add_messages_dir(std::string &&path) {
	m_messages_dir = std::move(path);
}

void mo_file_reader::add_mo_filename(const std::string &name) {
	m_mo_filename = name;
	if (m_mo_filename.substr(m_mo_filename.size() - 3) != ".mo")
		m_mo_filename += ".mo";
}

void mo_file_reader::add_mo_filename(std::string &&name) {
	m_mo_filename = std::move(name);
	if (m_mo_filename.substr(m_mo_filename.size() - 3) != ".mo")
		m_mo_filename += ".mo";
}

std::string mo_file_reader::gettext(const std::string &original_string) {
	try {
		return m_translation_map.at(original_string);
	}
	catch (const std::out_of_range &e) {
		return original_string;
	}
}

uint32_t mo_file_reader::read_section() {
	std::array<char, 4> buffer;
	m_ifstream.read(&buffer[0], buffer.size());
	uint32_t ret = 0;
	static_assert(sizeof(ret) == sizeof(char) * 4, "");
	std::memcpy(&ret, &buffer[0], buffer.size());
	return ret;
}


std::string mo_file_reader::get_system_lang_short_name() const {
#ifndef _WIN32
	std::string lang_env = std::getenv("LANG");
	if (lang_env.size() >= 2 && std::islower(lang_env.at(0)) && std::islower(lang_env.at(1)))
		return lang_env.substr(0, 2); // return first 2 characters (i.e. "pl" for "pl_PL.UTF-8")
	std::string locale = setlocale(LC_CTYPE, "");
#else
	char lang[3]; // ISO 639-1 == two-letter codes
	int ret = GetLocaleInfoA(
		LOCALE_USER_DEFAULT,
		LOCALE_SISO639LANGNAME,
		lang,
		sizeof(lang)
		);
	return std::string(lang);
#endif
}
