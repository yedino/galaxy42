// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP


#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include "mo_reader.hpp"

#include <mutex>


/// Current debug level. Plase change it only using g_dbg_level_set().
extern unsigned char g_dbg_level;


/// This macros will be moved later to glorious-cpp library or other

const char * dbg__FILE__(const char * name);

/// stack trace printers
///@{
std::string get_simple_backtrace(size_t depth=32); ///< Print regular stack trace with given depth

/// Print stack trace with given depth in a detailed way with references to source code
/// (works only if source code is available - require also compilation debug '-g' flag)
std::string get_detail_backtrace(size_t depth=32);
///@}

/**
 * @brief Change debug level to given one; Also it can mute/unmute further notifications about changes to debug level using it.
 * @param level: the level to be set. Or -1 to leave unchanged.
 * @param quiet: 0(false)/1(true)/-1(auto) - should we quiet reporting the level change.
 * @param quiet_from_now_on: 0(false)/1(true)/-1(unchanged) if it is 0 or 1 then it sets behaviour of quiet==-1(auto)
 * @note To be used in main program execution structure e.g. main(), or in tests / Unit Tests. Avoid using it from other code.
 * @note By default changes of debug level will be reported. To only stop reporting them use g_dbg_level_set(-1,"",-1,false);
 * And to only start reporting changes use g_dbg_level_set(-1,"",-1,true);
 * @codestyle This code avoids creating singleton object, so instead of a class, we have 1 function that does various things
 * and holds internal state in form of function static variables.
 */
void g_dbg_level_set(int level, std::string why, int quiet=-1, int quiet_from_now_on=-1);

/// gets the current debug level
int g_dbg_level_get();

#define _my__FILE__ (dbg__FILE__(__FILE__))

#define SHOW_DEBUG

#ifdef SHOW_DEBUG

extern const bool g_is_windows_console;

void write_to_console(const std::string& obj);

extern std::recursive_mutex g_dbg_mutex;

#define DBGLOCK std::lock_guard<std::recursive_mutex> dbg_lock( g_dbg_mutex );
#define DBGLVL(N) if (!(N>=g_dbg_level)) break

#define _main_dbg(X) { std::ostringstream _dbg_oss; _dbg_oss << X; write_to_console(_dbg_oss.str()); }

#define pfp_dbg5(X) do { } while(0)
#define pfp_dbg4(X) do { DBGLOCK DBGLVL(  5); _main_dbg("\033[90mdbg4: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl);} while(0)
#define pfp_dbg3(X) do { DBGLOCK DBGLVL( 10); _main_dbg("\033[37mdbg3: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl);} while(0)
#define pfp_dbg2(X) do { DBGLOCK DBGLVL( 20); _main_dbg("\033[37mdbg2: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl);} while(0)
#define pfp_dbg1(X) do { DBGLOCK DBGLVL( 30); _main_dbg("\033[97mdbg1: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl);} while(0)
#define pfp_info(X) do { DBGLOCK DBGLVL( 40); _main_dbg("\033[94minfo: " << _my__FILE__ << ':' << __LINE__ << " " << X  << "\033[0m" << ::std::endl);} while(0)
#define pfp_note(X) do { DBGLOCK DBGLVL( 50); _main_dbg("\033[36mnote: " << _my__FILE__ << ':' << __LINE__ << " " << X  << "\033[0m" << ::std::endl);} while(0)
#define pfp_clue(X) do { DBGLOCK DBGLVL( 50); _main_dbg("\n\033[96mclue: " << _my__FILE__ << ':' << __LINE__ << " " << X  << "\033[0m" << ::std::endl);} while(0)

#define pfp_fact_level(LVL_MAIN, LVL_EXTRA, X) \
	do { DBGLOCK; \
	DBGLVL(LVL_MAIN); \
	std::ostringstream _dbg_oss; \
	_dbg_oss<<"\033[92m"; \
	_dbg_oss<< X; \
	do { \
		DBGLVL(LVL_EXTRA); _dbg_oss << " (msg from " << _my__FILE__ << ':' << __LINE__ << ")"; } while(0); \
	_dbg_oss << "\033[0m" << ::std::endl; \
	write_to_console(_dbg_oss.str()); } while(0)
#define pfp_fact(X) pfp_fact_level(100, 30, X)
#define pfp_goal(X) pfp_fact_level(150, 30, X)

auto constexpr debug_config_warn_backtrace_level = 8; ///< the backtrace level used for pfp_warn
auto constexpr debug_config_erro_backtrace_level = 128; ///< the backtrace level used for pfp_erro

#define pfp_warn(X) \
	do { DBGLOCK; \
	DBGLVL(100); \
	std::ostringstream _dbg_oss; \
	_dbg_oss<<"\033[93m"; for (int i=0; i<70; ++i) _dbg_oss<<'!'; _dbg_oss<<::std::endl; \
	_dbg_oss<< ( "WARN:" ) << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; \
	write_to_console(_dbg_oss.str());\
	_dbg_oss.clear(); _dbg_oss.flush(); \
	_dbg_oss<<get_simple_backtrace( debug_config_warn_backtrace_level );\
	write_to_console(_dbg_oss.str());\
} while(0)
/// red code
//        ::std::cerr<<"ERROR! " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl;

#define pfp_erro(X) \
	do { DBGLOCK; \
	DBGLVL(200); \
	std::ostringstream _dbg_oss; \
	_dbg_oss<<"\033[91m\n"; for (int i=0; i<70; ++i) _dbg_oss<<'!'; _dbg_oss<<::std::endl; \
	_dbg_oss<< ("ERROR:") << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; \
	_dbg_oss<<"\n\n"; for (int i=0; i<70; ++i) _dbg_oss<<'!'; _dbg_oss<<"\033[0m"<<::std::endl; \
	write_to_console(_dbg_oss.str());\
	_dbg_oss.clear(); _dbg_oss.flush(); \
	_dbg_oss<<get_simple_backtrace( debug_config_erro_backtrace_level );\
	write_to_console(_dbg_oss.str());\
} while(0)

#define pfp_mark(X) \
	do { DBGLOCK; \
	DBGLVL(150); \
	std::ostringstream _dbg_oss; \
	_dbg_oss<<"\033[95m\n"; for (int i=0; i<70; ++i) _dbg_oss<<'='; _dbg_oss<<::std::endl; \
	_dbg_oss<<"MARK* " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; \
	for (int i=0; i<70; ++i) _dbg_oss<<'='; _dbg_oss<<"\033[0m"<<::std::endl; \
	write_to_console(_dbg_oss.str());\
	} while(0)

#else

#define pfp_dbg5(X) do {} while(0)
#define pfp_dbg4(X) do {} while(0)
#define pfp_dbg3(X) do {} while(0)
#define pfp_dbg2(X) do {} while(0)
#define pfp_dbg1(X) do {} while(0)
#define pfp_info(X) do {} while(0)
#define pfp_note(X) do {} while(0)
#define pfp_clue(X) do {} while(0)
#define pfp_fact(X) do {} while(0)
#define pfp_goal(X) do {} while(0)
#define pfp_warn(X) do {} while(0)
#define pfp_erro(X) do {} while(0)
#define pfp_mark(X) do {} while(0)

#endif


// TODO this is not really "debug", move to other file
#define pfp_UNUSED(x) (void)(x)

#define _NOTREADY() do { pfp_erro("This code is not implemented yet! in "<<__FUNCTION__);\
	throw std::runtime_error("Code not implemented yet! In: " + std::string(__FUNCTION__)); } while(0)

#define _NOTREADY_warn() do { pfp_warn("This code is not implemented yet! in "<<__FUNCTION__);\
	} while(0)

// this assert could be helpful, maybe use in release
#define pfp_assert(X) do { if (!(X)) { pfp_erro("Assertation failed (pfp_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

//        pfp_warn("Going to throw exception. What: " << except_var.what()
// this one is unused; leaving for translators if used again later.
#define pfp_unused_throw_error_msg \
	pfp_warn( mo_file_reader::gettext("L_what_exception_program_throw") << ": " << except_var.what() \

// TODO-r-deprecate: ?
#define pfp_throw_error_detail( EXCEPT , MSG ) do { auto except_var = (EXCEPT);  \
	pfp_warn( "Except: " << except_var.what() \
		<< "; Details:" << MSG); \
		throw except_var; } while(0)

#define pfp_throw_error( EXCEPT ) do { auto except_var = (EXCEPT);  \
	pfp_warn( "Except: " << except_var.what() \
		<< "."); \
		throw except_var; } while(0)

template <class T>
void throw_or_abort(const T & ex) {
#if defined(__GNUC__) && !defined(__clang__)
    #if __GNUC_PREREQ(6,0)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wterminate"
    #endif
#endif
	throw ex;
#if defined(__GNUC__) && !defined(__clang__)
    #if __GNUC_PREREQ(6,0)
        #pragma GCC diagnostic pop
    #endif
#endif
}

/**
*	This function tries to throw, BUT ALSO it is O.K. with possible ABORT, due to noexcept on the function we are in
*	it exists to e.g. silence some compilers warning about possible throw inside noexcept - we know, and we are o.k. with it turning into abort
*	use it in macros that should try to throw but abort if throwing is not possible
*/
#define pfp_throw_error_or_abort( EXCEPT ) do { auto except_var = (EXCEPT);  \
	pfp_warn( "Except: " << except_var.what() \
		<< "."); \
		throw_or_abort (except_var); } while(0)

#define pfp_throw_error_runtime( MSG ) pfp_throw_error( std::runtime_error( MSG ) )

namespace ui { class exception_error_exit; }

void must_be_exception_type_error_exit(const ui::exception_error_exit &x);

#define _throw_exit( EXCEPT ) do { auto except_var = (EXCEPT);  \
	must_be_exception_type_error_exit(except_var); /* assert (in compile time) that EXCEPT is of proper exception type */ \
	pfp_warn("Going to throw exception (for EXIT) What: " << except_var.what() \
		<< "."); \
		throw except_var; } while(0)

#define pfp_throw_error_rethrow( ) do { \
	pfp_warn("re-throw"); \
		throw ; } while(0)


// TODO-r-deprecate:
#define pfp_assert_throw(COND) do { \
	if (!(COND)) { \
		std::ostringstream _dbg_oss; \
		_dbg_oss << "Assert-throw failed: " << "" # COND ; \
		pfp_throw_error( std::runtime_error(_dbg_oss.str()) ); \
		}\
	} while(0)



#define UI_EXECUTE_OR_EXIT( FUNC ) \
				try { \
					try { \
			FUNC(); \
					} catch UI_CATCH_RETHROW( "" # FUNC ); \
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

#define _dbg5n(X) ;
#define pfp_dbg4n(X) pfp_dbg4(debug_this() << X)
#define pfp_dbg3n(X) pfp_dbg3(debug_this() << X)
#define pfp_dbg2n(X) pfp_dbg2(debug_this() << X)
#define pfp_dbg1n(X) pfp_dbg1(debug_this() << X)
#define pfp_infon(X) pfp_info(debug_this() << X)
#define pfp_noten(X) pfp_note(debug_this() << X)
#define pfp_cluen(X) pfp_clue(debug_this() << X)
#define pfp_factn(X) pfp_fact(debug_this() << X)
#define pfp_warnn(X) pfp_warn(debug_this() << X)
#define pfp_erron(X) pfp_erro(debug_this() << X)
#define pfp_markn(X) pfp_mark(debug_this() << X)

std::string to_string(const std::wstring &input); // TODO

#endif // include guard

