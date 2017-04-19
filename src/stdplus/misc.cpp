#include <stdplus/misc.hpp>


namespace stdplus {


std::string to_string(const std::string & v) {
	return v;
}


const char* expected_exception::what() const noexcept {
	return "expected_exception";
}

const char* expected_not_found::what() const noexcept {
	return "expected_not_found";
}


}
