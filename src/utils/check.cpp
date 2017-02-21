#include "utils/check.hpp"

const char * err_check_soft::what() {
	auto ptr = dynamic_cast< err_check_base * >( this );
	if (ptr) return ptr->what();
	return "Undefined soft error (huh?)";
}

// -------------------------------------------------------------------

err_check_base::err_check_base(tag_err_check_named, const char   * what, bool serious)
	: std::runtime_error(what) ,  m_serious(serious)  { }
err_check_base::err_check_base(tag_err_check_named, const string & what, bool serious)
	: std::runtime_error(what) ,  m_serious(serious)  { }

bool err_check_base::is_serious() const { return m_serious; }

// -------------------------------------------------------------------

err_check_user::err_check_user(const char *what)
	: err_check_base(tag_err_check_named{} , cause()+what , true)  { }
err_check_user::err_check_user(tag_err_check_named, const char   * what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
err_check_user::err_check_user(tag_err_check_named, const string & what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
std::string err_check_user::cause() {
	return "Check detected user error: "s;
}

// -------------------------------------------------------------------

err_check_user_soft::err_check_user_soft(const char *what)
	: err_check_user(tag_err_check_named{} , cause()+what , true) { }
std::string err_check_user_soft::cause() {
	return "Check detected user warning: "s;
}

// -------------------------------------------------------------------

