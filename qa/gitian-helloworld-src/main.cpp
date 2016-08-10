
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

// #include <json/json.h>

#include <sodium.h>

void foo() {
	std::cout << "foo" << std::endl;
}

int bar();
int baz();

int main() {
	foo();

	if (sodium_init() == -1) {
		return 1;
	}

	foo();
	auto a = bar();
	auto b = baz();
	std::cout << a+b << std::endl;
}

