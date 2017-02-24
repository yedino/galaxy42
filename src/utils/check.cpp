#include "utils/check.hpp"

const char * err_check_soft::what() const {
	auto ptr = dynamic_cast< const err_check_base * >( this );
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

err_check_prog::err_check_prog(const char *what)
	: err_check_base(tag_err_check_named{} , cause(true)+what , true)  { }

err_check_prog::err_check_prog(tag_err_check_named, const char   * what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
err_check_prog::err_check_prog(tag_err_check_named, const string & what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }

std::string err_check_prog::cause(bool se) {
	if (se)	return "Check detected prog ERROR: "s;
	return "Check detected prog warning: "s;
}

// -------------------------------------------------------------------

err_check_user::err_check_user(const char *what)
	: err_check_base(tag_err_check_named{} , cause(true)+what , true)  { }

err_check_user::err_check_user(tag_err_check_named, const char   * what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
err_check_user::err_check_user(tag_err_check_named, const string & what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }

std::string err_check_user::cause(bool se) {
	if (se)	return "Check detected user ERROR: "s;
	return "Check detected user warning: "s;
}

err_check_user_soft::err_check_user_soft(const char *what)
	: err_check_user(tag_err_check_named{} , cause(false)+what , false) { }

// -------------------------------------------------------------------

err_check_sys::err_check_sys(const char *what)
	: err_check_base(tag_err_check_named{} , cause(true)+what , true)  { }
err_check_sys::err_check_sys(tag_err_check_named, const char   * what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
err_check_sys::err_check_sys(tag_err_check_named, const string & what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
std::string err_check_sys::cause(bool se) {
	if (se) return "Check detected system ERROR: "s;
	return "Check detected system warning: "s;
}

err_check_sys_soft::err_check_sys_soft(const char *what)
	: err_check_sys(tag_err_check_named{} , cause(false)+what , false) { }

// -------------------------------------------------------------------

err_check_extern::err_check_extern(const char *what)
	: err_check_base(tag_err_check_named{} , cause(true)+what , true)  { }
err_check_extern::err_check_extern(tag_err_check_named, const char   * what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
err_check_extern::err_check_extern(tag_err_check_named, const string & what, bool serious)
	: err_check_base(tag_err_check_named{} , what , serious) { }
std::string err_check_extern::cause(bool se) {
	if (se) return "Check detected EXTERNALLY-CAUSED ERROR: "s;
	return "Check detected externally-caused warning: "s;
}

err_check_extern_soft::err_check_extern_soft(const char *what)
	: err_check_extern(tag_err_check_named{} , cause(false)+what , false) { }

// -------------------------------------------------------------------

