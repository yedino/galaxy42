// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "c_tnetdbg.hpp"

#include <string>
#include <cstring>

unsigned char g_dbg_level = 100; // (extern)

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
//#include <winbase.h>
//#include <wincon.h>
#include <stringapiset.h>
#undef _assert
#include <cstring>

void set_windows_console_utf8(HANDLE console_handle) {
	CONSOLE_FONT_INFOEX cfi = {sizeof(cfi)};
	GetCurrentConsoleFontEx(console_handle, FALSE, &cfi);
	wchar_t font_name[32] = L"Lucida Console";
	std::memcpy(cfi.FaceName, font_name, 32);
	SetCurrentConsoleFontEx(console_handle, FALSE, &cfi);
	// SetConsoleOutputCP function not works
	std::system("chcp 65001 > nul");
	bool chcp_ret = (GetConsoleOutputCP() == CP_UTF8);
	if (!chcp_ret) {
		throw::std::runtime_error("SetConsoleOutputCP error, nr: " + std::to_string(GetLastError()));
	}
}
#endif

const bool g_is_windows_console = []() {
	#if defined(_WIN32) || defined(__CYGWIN__)
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD n = 0;
		bool ret = WriteConsole(hConsole, nullptr, 0, &n, nullptr);
		if (ret) { // if windows console
			set_windows_console_utf8(hConsole);
		}
		return ret;
	#else
		return false;
	#endif
}();

#include <cassert>
void write_to_console(const std::string& obj) {
	if (g_is_windows_console) {
		#if defined(_WIN32) || defined(__CYGWIN__)
		std::ostringstream oss;
		oss << obj;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		const std::string str = oss.str();
		wchar_t buf[1024];
		DWORD write_bytes = 0;
		write_bytes = MultiByteToWideChar(CP_UTF8, 0,
            str.c_str(), str.size()+1, // +1 for '\0'
            buf, sizeof(buf));
		::std::wcerr << buf;
		#else
			assert(false); // windows console detected on non windows OS
		#endif
	}
	else
		::std::cerr << obj;
}

void g_dbg_level_set(unsigned char level, std::string why, bool quiet) {
	bool more_debug = level < g_dbg_level;
	if ( more_debug) g_dbg_level = level; // increase before printing
	if (!quiet) {
		_note("Setting debug level to " << static_cast<int>(level) << " because: " << why);
	}
	if (!more_debug) g_dbg_level = level; // increase after printing
}


const char * dbg__FILE__(const char * name) {
	const char * s_target = "/galaxy42/";
	size_t l_target = strlen(s_target);
	const char * pos_target  = std::strstr(name, s_target);
	if (pos_target==nullptr) return name; // not matched
	const char * s_src = "/src/";
	size_t l_src = strlen(s_src);
	const char * pos_src = std::strstr(pos_target, s_src);
	// std::cout << pos_src << "<------------" << std::endl;
	if (pos_src==nullptr) return pos_target;
	return pos_src + l_src; // convert "/src/x.cpp" to "x.cpp"
}


void must_be_exception_type_error_exit(const ui::exception_error_exit &x)
{ _UNUSED(x); }

std::string debug_this() {
	return "(global) ";
}

// TODO
std::string to_string(const std::wstring &input) {
	std::string ret;
	for (const auto & it : input)
		ret += it;
	return ret;
}
