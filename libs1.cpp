
#include "libs1.hpp"


const char* expected_exception::what() const noexcept {
	return "expected_exception";
}


const char* expected_not_found::what() const noexcept {
	return "expected_not_found";
}

