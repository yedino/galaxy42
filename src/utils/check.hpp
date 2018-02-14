#pragma once

#include <string>
#include <memory> // for UsePtr
#include <vector>

#include <limits>

#include <tnetdbg.hpp>

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
 * @brief A general class to catch try_soft errors (errors that are very expected).
 * @note use this in catch(...) to catch all soft errors
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

/**
	* Base of all exceptions thrown by our _check system.
	* @note better do NOT use it in any catch directly
	*/
class err_check_base : public std::runtime_error {
	protected:
		const bool m_serious;
	public:
		err_check_base(tag_err_check_named, const char   * what, bool serious);
		err_check_base(tag_err_check_named, const std::string & what, bool serious);
		bool is_serious() const;
};
// -------------------------------------------------------------------

/**
 * This class is for exeption representing: programming error. This is fault in program execution.
 * This is always a hard error (as there is no child class for soft variant of it).
 * @note use this in catch(...) to catch the hard error from _check()
 */
class err_check_prog : public err_check_base {
	public:
		err_check_prog(const char *what); ///< create hard error, from this message (can add cause string)
	protected:
		/// for use by child class where the child class generated entire message
		///@{
		err_check_prog(tag_err_check_named, const char   * what, bool serious);
		err_check_prog(tag_err_check_named, const std::string & what, bool serious);
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
		err_check_user(tag_err_check_named, const std::string & what, bool serious);
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
		err_check_input(tag_err_check_named, const std::string & what, bool serious);
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
		err_check_sys(tag_err_check_named, const std::string & what, bool serious);
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
		err_check_extern(tag_err_check_named, const std::string & what, bool serious);
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
#define _check(X) do { if(!(X)) { _throw_error_or_abort( err_check_prog( #X ) );  } } while(0)

/// Macro that checks arg X, throws err_check_user if false
#define _check_user(X) do { if(!(X)) { _throw_error_or_abort( err_check_user( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_input if false
#define _check_input(X) do { if(!(X)) { _throw_error_or_abort( err_check_input( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_sys if false
#define _check_sys(X) do { if(!(X)) { _throw_error_or_abort( err_check_sys( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_extern if false
#define _check_extern(X) do { if(!(X)) { _throw_error_or_abort( err_check_extern( #X ) );  } } while(0)

/// Macro that checks arg X, throws err_check_user_soft if false
#define _try_user(X) do { if(!(X)) { _throw_error( err_check_user_soft( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_input_soft if false
#define _try_input(X) do { if(!(X)) { _throw_error( err_check_input_soft( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_sys_soft if false
#define _try_sys(X) do { if(!(X)) { _throw_error( err_check_sys_soft( #X ) );  } } while(0)
/// Macro that checks arg X, throws err_check_extern_soft if false
#define _try_extern(X) do { if(!(X)) { _throw_error( err_check_extern_soft( #X ) );  } } while(0)


// always abort - for serious errors
#define _check_abort(X) do { if (!(X)) { \
	_erro("Assertation failed, will abort: (" << #X << ")" << _my__FILE__ << ':' << __LINE__); \
	::std::abort(); } \
} while(0)

// -------------------------------------------------------------------

template<class T> T& _UsePtr(const std::shared_ptr<T> & ptr, int line, const char* file) {
	if (!ptr) { _erro("Failed pointer, for " << file << ":" << line);
		std::abort();
	}
	return *ptr;
}

template<class T> T& _UsePtr(const std::unique_ptr<T> & ptr, int line, const char* file) {
	if (!ptr) { _erro("Failed pointer, for " << file << ":" << line);
		std::abort();
	}
	return *ptr;
}


// -------------------------------------------------------------------

/// Where to run invariant check / which invariant (pre,post,both,...)
typedef enum {
	e_invariant_place_pre, ///< run check only before function main body
	e_invariant_place_post, ///< run check only after function main body
	e_invariant_place_both, ///< run all checks
} t_invariant_place;

/// This is a RAII that calls Precond() and Postcond() for an object. Use it like a lock-guard in your member functions.
/// @owner rafal
template<typename TC> class c_ig final {
	public:
		c_ig(const TC & thisobj, t_invariant_place place, int fromline, const char* fromfile)
		: m_thisobj(thisobj), m_place(place), m_fromline(fromline), m_fromfile(fromfile)
		{
			if (!fromfile) { _erro("Invalid fromfile used in debug."); fromfile="(unknown source file)"; }
			if ( (place==e_invariant_place_both) || (place==e_invariant_place_pre) ) {
				try {
					m_thisobj.Precond();
				} catch(...) {
					_erro("Pre-condition check caused error, for precondition from: "
						<< m_fromfile << ":" << m_fromline
					);
					_check_abort(0);
				}
			}
		}

		~c_ig() {
			if ( (m_place==e_invariant_place_both) || (m_place==e_invariant_place_pre) ) {
				try {
					m_thisobj.Postcond();
				} catch(...) {
					_erro("Post-condition check caused error, for precondition from: "
						<< m_fromfile << ":" << m_fromline
					);
					_check_abort(0);
				}
			}
		}

	private:
		const TC & m_thisobj;
		t_invariant_place m_place;
		int m_fromline;
		const char* m_fromfile;
};

template<typename TC> c_ig<TC> make_ig(const TC & thisobj, t_invariant_place place, int fromline, const char * fromfile) {
	return c_ig<TC>(thisobj,place,fromline,fromfile);
}

#define guard_inv auto invariant_guard_obj = make_ig(*this,e_invariant_place_both,__LINE__,__FILE__)

#define guard_inv_post auto invariant_guard_obj = make_ig(*this,e_invariant_place_post,__LINE__,__FILE__)

#define guard_inv_pre  auto invariant_guard_obj = make_ig(*this,e_invariant_place_pre,__LINE__,__FILE__)
// ^-- in this case we could all together skip creation of the object. unless we want it for debug? Let's see what works.

// -------------------------------------------------------------------

#define guard_LOCK(MUTEX)	std::lock_guard< decltype(MUTEX) > lg(MUTEX)

// -------------------------------------------------------------------

// -------------------------------------------------------------------
/// @name reasonable
/// @brief We want to check that objects we operate on stay in reasonable size.
/// For example std::string does it more-or-less as that: size of std::string must be less then /2 or /4 of size_t
/// after doing that one check, we know that all operations like string1+string2 will also be fitting inside the limit.
///
/// Here we configure limits for our program, how ever we set them much lower:
/// In this project, objects of reasonable size, are ones that:
/// 1) fit in expected memory limits
/// 2) operating on them in expected typical way will not quickly make them unreasonable
///
/// This functions will check against this, and e.g. throw exception.
/// They also can disaply warnings once we are getting closer to limit.
/// @{
constexpr size_t reasonable_size_limit_bytes    =  32 * 1024 * 1024; ///< objects should never get [nearly] this big
constexpr size_t reasonable_size_limit_elements =   4 * 1000 * 1000; ///< objects should never get [nearly] that many elements
constexpr size_t reasonable_size_mul = 4; ///< we expects objects to grow this (e.g. 4) times more over the limit
constexpr size_t reasonable_size_mul_warning = 2; ///< display warning once objects are only N times smaller then the limit

constexpr size_t reasonable_size_limit_bytes_divided_max  = reasonable_size_limit_bytes / reasonable_size_mul;
constexpr size_t reasonable_size_limit_bytes_divided_warn = reasonable_size_limit_bytes / reasonable_size_mul_warning;
constexpr size_t reasonable_size_limit_elements_divided_max  = reasonable_size_limit_elements / reasonable_size_mul;
constexpr size_t reasonable_size_limit_elements_divided_warn = reasonable_size_limit_elements / reasonable_size_mul_warning;

void reasonable_size(const std::string & obj); ///< will throw if string is unreasonably big, see #reasonable

template<typename T> void reasonable_size(const T & obj) { ///< will throw if this some container is too big, see #reasonable
	const size_t elements = obj.size();
	if (! (elements < reasonable_size_limit_elements_divided_warn) ) {
		_warn("Object @"<<static_cast<const void*>(&obj)<<" starts to get too big: elemens="<<elements);
	}
	_check_input(elements <= reasonable_size_limit_elements_divided_max);

	const size_t bytes = elements * sizeof(typename T::value_type);
	if (! (bytes < reasonable_size_limit_bytes_divided_warn) ) {
		_warn("Object @"<<static_cast<const void*>(&obj)<<" starts to get too big: bytes="<<bytes);
	}
	_check_input(bytes <= reasonable_size_limit_bytes_divided_max);
}

static_assert( reasonable_size_limit_bytes >= reasonable_size_limit_elements ,
	"Allowed limit of bytes should be >= then limit of elements");

static_assert( reasonable_size_limit_bytes_divided_max < std::numeric_limits<size_t>::max()/8 , "use lower limit");
static_assert( reasonable_size_limit_bytes_divided_max < std::numeric_limits<int>::max()/4 , "use lower limit");
static_assert( reasonable_size_limit_bytes_divided_max+1 < std::numeric_limits<int>::max()/4 , "use lower limit");

static_assert( reasonable_size_limit_elements_divided_max < std::numeric_limits<size_t>::max()/8 , "use lower limit");
static_assert( reasonable_size_limit_elements_divided_max < std::numeric_limits<int>::max()/4 , "use lower limit");
static_assert( reasonable_size_limit_elements_divided_max+1 < std::numeric_limits<int>::max()/4 , "use lower limit");

// }@
// -------------------------------------------------------------------


#define UsePtr(PTR) _UsePtr(PTR,__LINE__,__FILE__)

