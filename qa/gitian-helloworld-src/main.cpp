
#include <iostream>

#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int/limits.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/program_options.hpp>

#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <streambuf>
#include <string>
#include <string.h>

#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>

//#include <json/json.h>

#include <sodium.h>

// #include <sodiumpp/sodiumpp.h>

void foo() {
	std::cout << "foo" << std::endl;
}
/*
void print_compilation_data() {
	std::cout << "TIME = " << __TIME__ << std::endl;
	std::cout << "DATE = " << __DATE__ << std::endl;
	std::cout << "FILE = " << __FILE__ << std::endl;
	std::cout << "FUNCTION = " << __FUNCTION__ << std::endl;
}
*/
int bar();
int baz();

/*
void use_sodiumpp() {
	std::cout << "Using sodiumpp" << std::endl;

	using sodiumpp::locked_string;
	locked_string s1(50);
	locked_string s2=s1;
	locked_string s3=std::move(s1);
}
*/
/*
void use_json() {
	std::cout << "Using JSON" << std::endl;

	Json::Value m_root;
	try {
		Json::Value authpass_array = m_root["authorizedPasswords"];
	} catch(...) {
	}
}
*/
int main() {
	foo();

	if (sodium_init() == -1) {
		return 1;
	}


	foo();
	auto a = bar();
	auto b = baz();
	std::cout << a+b << std::endl;

//	use_sodiumpp();
//	use_json();
//	print_compilation_data();

	std::cout << "All ok" << std::endl;
}

