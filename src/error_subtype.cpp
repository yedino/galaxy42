

#include "error_subtype.hpp"

// TODO move

runtime_error_subtype::runtime_error_subtype()
: std::runtime_error("Some subtype of runtime error (call our virtual .what function to see details - catch this exception by reference/pointer to std::exception)")
// this default message is for case if someone would catch (runtime_error_subtype except) instead using refernece catch (runtime_error_subtype &except)
{
}

const char * runtime_error_subtype::what() const noexcept {
	return m_msg.c_str();
}

void runtime_error_subtype::set_msg(const std::string &msg) {
	m_msg = msg;
}


