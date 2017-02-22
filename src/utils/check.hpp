#pragma once

#include "libs1.hpp"

/***
@file Check is a library part that offers macros that do certain checks and assertions, but can
result in exception being nicelly thrown.
*/

struct tag_err_check_named{}; ///< tag for constructor of err_check_* saying that the name of object is already decided

/// A general class to catch soft errors. It can only be used to inherit to some class that also inherits from err_check_base
/// use case is that you catch (err_check_soft &ex) and you can then still ex->what() propertly (it will dynamic cast itself
/// and get the error message from err_check_base)
class err_check_soft {
	public:
		virtual const char * what(); ///< return the error message, like from std::runtime_error::what()
};

/// base of all exceptions thrown by our _check system
class err_check_base : public std::runtime_error {
	protected:
		const bool m_serious;
	public:
		err_check_base(tag_err_check_named, const char   * what, bool serious);
		err_check_base(tag_err_check_named, const string & what, bool serious);
		bool is_serious() const;
};

// -------------------------------------------------------------------

/// This class is for exeption representing: user error. It can be (and is by default) a hard error.
class err_check_user : public err_check_base {
	public:
		err_check_user(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		err_check_user(tag_err_check_named, const char   * what, bool serious); ///< for use by child class where the child class generated entire message
		err_check_user(tag_err_check_named, const string & what, bool serious); ///< for use by child class where the child class generated entire message
		static std::string cause(bool se);
};

class err_check_user_soft final : public err_check_user, public err_check_soft {
	public:
		err_check_user_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

/// This class is for exeption representing: sys error. It can be (and is by default) a hard error.
class err_check_sys : public err_check_base {
	public:
		err_check_sys(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		err_check_sys(tag_err_check_named, const char   * what, bool serious); ///< for use by child class where the child class generated entire message
		err_check_sys(tag_err_check_named, const string & what, bool serious); ///< for use by child class where the child class generated entire message
		static std::string cause(bool se);
};

class err_check_sys_soft final : public err_check_sys, public err_check_soft {
	public:
		err_check_sys_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

/// This class is for exeption representing: extern error. It can be (and is by default) a hard error.
class err_check_extern : public err_check_base {
	public:
		err_check_extern(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		err_check_extern(tag_err_check_named, const char   * what, bool serious); ///< for use by child class where the child class generated entire message
		err_check_extern(tag_err_check_named, const string & what, bool serious); ///< for use by child class where the child class generated entire message
		static std::string cause(bool se);
};

class err_check_extern_soft final : public err_check_extern, public err_check_soft {
	public:
		err_check_extern_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

#define _check(X) do { if(!(X)) { throw err_check_prog( #X );  } } while(0)

#define _check_user(X) do { if(!(X)) { throw err_check_user( #X );  } } while(0)
#define _check_sys(X) do { if(!(X)) { throw err_check_sys( #X );  } } while(0)
#define _check_extern(X) do { if(!(X)) { throw err_check_extern( #X );  } } while(0)

#define _try_user(X) do { if(!(X)) { throw err_check_user_soft( #X );  } } while(0)
#define _try_sys(X) do { if(!(X)) { throw err_check_sys_soft( #X );  } } while(0)
#define _try_extern(X) do { if(!(X)) { throw err_check_extern_soft( #X );  } } while(0)

