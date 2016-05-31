#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP

#include <cstdlib>

#include <string>


/// This macros will be moved later to glorious-cpp library or other

const char * debug_shorten__FILE__(const char * name);

#define _my__FILE__ (debug_shorten__FILE__(__FILE__))

#define SHOW_DEBUG
#ifdef SHOW_DEBUG

#define _dbg3(X) do { ::std::cerr<<"dbg3: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg2(X) do { ::std::cerr<<"dbg2: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg1(X) do { ::std::cerr<<"dbg1: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
#define _info(X) do { ::std::cerr<<"\033[94minfo: " << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; } while(0)	///< blue esc code
#define _note(X) do { ::std::cerr<<"note: " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; } while(0)
/// yellow code
#define _warn(X) do { \
	::std::cerr<<"\033[93m\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<<"Warn! " << _my__FILE__ << ':' << __LINE__ << " " << X << "\033[0m" << ::std::endl; \
} while(0)
/// red code
#define _erro(X) do { \
	::std::cerr<<"\033[91m\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<<"DAMN! " << _my__FILE__ << ':' << __LINE__ << " " << X << ::std::endl; \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<"\033[0m"<<::std::endl; \
} while(0)
#define _mark(X) do { \
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
	throw std::runtime_error("Code not implemented yet! In: " + string(__FUNCTION__)); } while(0)

#define _NOTREADY_warn() do { _warn("This code is not implemented yet! in "<<__FUNCTION__);\
	} while(0)


// this assert is probably not important, rather only in debug
#define _obvious(X) do { if (!(X)) { _erro("Assertation failed (_obvious) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); } } while(0)

// this assert could be helpful, maybe use in release
#define _assert(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _throw_error(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << _my__FILE__ << ':' << __LINE__); ::std::abort(); }  } while(0)

#define _throw2( EXCEPT , MSG ) do { auto except_var = EXCEPT;  _erro("Going to throw exception. What:" << except_var.what() << " Info:" << MSG); throw except_var; } while(0)
#define _throw( EXCEPT ) _throw2(EXCEPT, "")


/** TODO document
throw std::logic_error("foo");
throw _except2( std::logic_error("foo") , "x=" << x << "y=" << y ) ;
throw _except( std::logic_error("foo")) ;
*/


std::string debug_this() {
	return "(global)";
}

#define _dbg3n(X) _dbg3(debug_this() << X)
#define _dbg2n(X) _dbg2(debug_this() << X)
#define _dbg1n(X) _dbg1(debug_this() << X)
#define _infon(X) _info(debug_this() << X)
#define _noten(X) _note(debug_this() << X)
#define _warnn(X) _warn(debug_this() << X)
#define _erron(X) _erro(debug_this() << X)
#define _markn(X) _mark(debug_this() << X)



#endif // include guard

