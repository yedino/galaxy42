#pragma once

#include "libs1.hpp"

/**
 * @file check.hpp
 * @brief Library for errors exception and assertions
 *
 * Check is a library part that offers macros that do certain checks and assertions, but can
 * result in exception being nicelly thrown.
 */

/// tag for constructor of err_check_* saying that the name of object is already decided
struct tag_err_check_named{};

/**
 * @brief A general class to catch soft errors.
 *
 * It can only be used to inherit to some class that also inherits from err_check_base
 * use case is that you catch (err_check_soft &ex) and you can then still ex->what() propertly (it will dynamic cast itself
 * and get the error message from err_check_base)
 */
class err_check_soft {
	public:
		virtual const char * what_soft() const; ///< use this if you catch as err_check_soft exactly; else use normal what()
	protected:
		err_check_soft() = default;
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

/// This class is for exeption representing: programming error. This is fault in program execution.
/// This errors are always hard errors.
class err_check_prog : public err_check_base {
	public:
		err_check_prog(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		/// for use by child class where the child class generated entire message
		///@{
		err_check_prog(tag_err_check_named, const char   * what, bool serious);
		err_check_prog(tag_err_check_named, const string & what, bool serious);
		///@}
		static std::string cause(bool se);
};

// -------------------------------------------------------------------

/// This class is for exeption representing: user error. It can be (and is by default) a hard error.
class err_check_user : public err_check_base {
	public:
		err_check_user(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		/// for use by child class where the child class generated entire message
		///@{
		err_check_user(tag_err_check_named, const char   * what, bool serious);
		err_check_user(tag_err_check_named, const string & what, bool serious);
		///@}
		static std::string cause(bool se);
};

/// Class that represents soft case of err_check_user
class err_check_user_soft final : public err_check_user, public err_check_soft {
	public:
		err_check_user_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

/// This class is for exeption representing: input error. It can be (and is by default) a hard error.
class err_check_input : public err_check_base {
	public:
		err_check_input(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		/// for use by child class where the child class generated entire message
		///@{
		err_check_input(tag_err_check_named, const char   * what, bool serious);
		err_check_input(tag_err_check_named, const string & what, bool serious);
		///@}
		static std::string cause(bool se);
};

/// Class that represents soft case of err_check_input
class err_check_input_soft final : public err_check_input, public err_check_soft {
	public:
		err_check_input_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

/// This class is for exeption representing: sys error. It can be (and is by default) a hard error.
class err_check_sys : public err_check_base {
	public:
		err_check_sys(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		/// for use by child class where the child class generated entire message
		///@{
		err_check_sys(tag_err_check_named, const char   * what, bool serious);
		err_check_sys(tag_err_check_named, const string & what, bool serious);
		///@}
		static std::string cause(bool se);
};

/// Class that represents soft case of err_check_sys
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
		/// for use by child class where the child class generated entire message
		///@{
		err_check_extern(tag_err_check_named, const char   * what, bool serious);
		err_check_extern(tag_err_check_named, const string & what, bool serious);
		///@}
		static std::string cause(bool se);
};

/// Class that represents soft case of err_check_extern
class err_check_extern_soft final : public err_check_extern, public err_check_soft {
	public:
		err_check_extern_soft(const char *what); ///< create soft error, from this message (can add cause string)
};

// -------------------------------------------------------------------

/// err_check_prog?
#define _check(X) do { if(!(X)) { throw err_check_prog( #X );  } } while(0)

/// Macro that checks arg X, throws err_check_user if false
#define _check_user(X) do { if(!(X)) { throw err_check_user( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_input if false
#define _check_input(X) do { if(!(X)) { throw err_check_input( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_sys if false
#define _check_sys(X) do { if(!(X)) { throw err_check_sys( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_extern if false
#define _check_extern(X) do { if(!(X)) { throw err_check_extern( #X );  } } while(0)

/// Macro that checks arg X, throws err_check_user_soft if false
#define _try_user(X) do { if(!(X)) { throw err_check_user_soft( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_input_soft if false
#define _try_input(X) do { if(!(X)) { throw err_check_input_soft( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_sys_soft if false
#define _try_sys(X) do { if(!(X)) { throw err_check_sys_soft( #X );  } } while(0)
/// Macro that checks arg X, throws err_check_extern_soft if false
#define _try_extern(X) do { if(!(X)) { throw err_check_extern_soft( #X );  } } while(0)
