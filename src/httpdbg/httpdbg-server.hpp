
#pragma once
#ifndef HTTPDBG_HTTPDBG_SERVER
#define HTTPDBG_HTTPDBG_SERVER 1

#include <thread>
#include <mutex>

class c_tunserver;

/**
 * Read data from c_tunserver, generate a string (e.g. with HTML) that shows it's debug info.
*/
class c_httpdbg_raport {
	public:
		c_httpdbg_raport(const c_tunserver & target);
		std::string generate();

	protected:
		const c_tunserver & m_target;

		static std::string HTML(const std::string & s);

		template <class T>
		static std::string HTML(const T & obj);
};

template <class T>
std::string c_httpdbg_raport::HTML(const T & obj) {
	std::ostringstream out;
	out << obj;
	return HTML( out.str() ); // use the normal-string version of our function
}

int main_httpdbg(int opt_port, const c_tunserver & tunserver);

#endif

