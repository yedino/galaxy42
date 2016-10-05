// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <libintl.h>

extern unsigned char g_dbg_level;


/// This macros will be moved later to glorious-cpp library or other

const char * debug_shorten__FILE__(const char * name);

void g_dbg_level_set(unsigned char level, std::string why, bool quiet=false);

#define _my__FILE__ (debug_shorten__FILE__(__FILE__))

#define SHOW_DEBUG
#ifdef SHOW_DEBUG

#define DBGLVL(N) if (!(N>=g_dbg_level)) break

#define _dbg3(X) do { DBGLVL( 10); ::std::cerr<<"dbg3: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg2(X) do { DBGLVL( 20); ::std::cerr<<"dbg2: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg1(X) do { DBGLVL( 30); ::std::cerr<<"dbg1: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _info(X) do { DBGLVL( 40); ::std::cerr<<"\033[94minfo: " << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; } while(0)	///< blue esc code
#define _note(X) do { DBGLVL( 50); ::std::cerr<<"note: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
/// yellow code
//        ::std::cerr<<"Warn! " << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; 

#define _warn(X) do { DBGLVL(100); \
	::std::cerr<<"\033[93m\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<< gettext( "L_warn" ) << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; \
} while(0)
/// red code
//        ::std::cerr<<"ERROR! " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; 

#define _erro(X) do { DBGLVL(200); \
	::std::cerr<<"\033[91m\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<<gettext("L_error") << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<"\033[0m"<<::std::endl; \
} while(0)
#define _mark(X) do { DBGLVL(150); \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	::std::cerr<<"MARK* " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; \
	for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	} while(0)

#else

#define _dbg3(X) do {} while(0)
#define _dbg2(X) do {} while(0)
#define _dbg1(X) do {} while(0)
#define _info(X) do {} while(0)
#define _note(X) do {} while(0)
#define _warn(X) do {} while(0)
#define _erro(X) do {} while(0)
#define _mark(X) do {} while(0)

#endif

// TODO this is not really "debug", move to other file
#define _UNUSED(x) (void)(x)

#define _NOTREADY() do { _erro("This code is not implemented yet! in "<<__FUNCTION__);\
	throw std::runtime_error("Code not implemented yet! In: " + std::string(__FUNCTION__)); } while(0)

#define _NOTREADY_warn() do { _warn("This code is not implemented yet! in "<<__FUNCTION__);\
	} while(0)


// this assert is probably not important, rather only in debug
#define _obvious(X) do { if (!(X)) { _erro("Assertation failed (_obvious) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); } } while(0)

// this assert could be helpful, maybe use in release
#define _assert(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)


//        _warn("Going to throw exception. What: " << except_var.what()
#define _throw_error_2( EXCEPT , MSG ) do { auto except_var = EXCEPT;  \
	_warn( gettext("L_what_exception_program_throwow") << except_var.what() \
		<< "; Details:" << MSG); \
		throw except_var; } while(0)

//        _warn("Going to throw exception. What: " << except_var.what() 

#define _throw_error( EXCEPT ) do { auto except_var = EXCEPT;  \
	_warn(gettext("L_what_exception_program_throwow") << except_var.what() \
		<< "."); \
		throw except_var; } while(0)


namespace ui { class exception_error_exit; }

void must_be_exception_type_error_exit(const ui::exception_error_exit &x);

#define _throw_exit( EXCEPT ) do { auto except_var = EXCEPT;  \
	must_be_exception_type_error_exit(except_var); /* assert (in compile time) that EXCEPT is of proper exception type */ \
	_warn("Going to throw exception (for EXIT) What: " << except_var.what() \
		<< "."); \
		throw except_var; } while(0)

#define _throw_error_rethrow( ) do { \
	_warn("Going to re-throw exception."); \
		throw ; } while(0)

#define _assert_throw(COND) do { \
	if (!(COND)) { \
		std::ostringstream oss; \
		oss << "Assert-throw failed: " << "" # COND ; \
		_throw_error( std::runtime_error(oss.str()) ); \
		}\
	} while(0)



#define UI_EXECUTE_OR_EXIT( FUNC ) \
				try { \
					try { \
			FUNC(); \
					} UI_CATCH_RETHROW( "" # FUNC ); \
				} catch(...) { \
					std::cout << "Error occured (see above) - we will exit now" << std::endl; \
					_throw_exit( ui::exception_error_exit("Error in: " # FUNC ) ); \
				}




/** TODO document
throw std::logic_error("foo");
throw _except2( std::logic_error("foo") , "x=" << x << "y=" << y ) ;
throw _except( std::logic_error("foo")) ;
*/

std::string debug_this();

#define _dbg3n(X) _dbg3(debug_this() << X)
#define _dbg2n(X) _dbg2(debug_this() << X)
#define _dbg1n(X) _dbg1(debug_this() << X)
#define _infon(X) _info(debug_this() << X)
#define _noten(X) _note(debug_this() << X)
#define _warnn(X) _warn(debug_this() << X)
#define _erron(X) _erro(debug_this() << X)
#define _markn(X) _mark(debug_this() << X)



#endif // include guard

