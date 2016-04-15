#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP

#include <cstdlib>


/// This macros will be moved later to glorious-cpp library or other

#define _dbg3(X) do{}while(0)
// do { ::std::cerr<<"dbg3: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)

#define _dbg2(X) do { ::std::cerr<<"dbg2: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg1(X) do { ::std::cerr<<"dbg1: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _info(X) do { ::std::cerr<<"info: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _note(X) do { ::std::cerr<<"note: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _warn(X) do { \
	::std::cerr<<"\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<<"Warn! " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; \
} while(0)
#define _erro(X) do { \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
	::std::cerr<<"ERRO! " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'!'; ::std::cerr<<::std::endl; \
} while(0)
#define _mark(X) do { \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	::std::cerr<<"MARK* " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; \
	::std::cerr<<"\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	} while(0)

// TODO this is not really "debug", move to other file
#define _UNUSED(x) (void)(x)

#define _NOTREADY() do { _erro("This code is not implemented yet! in "<<__FUNCTION__);\
	throw std::runtime_error("Code not implemented yet! In: " + string(__FUNCTION__)); } while(0)

#define _NOTREADY_warn() do { _warn("This code is not implemented yet! in "<<__FUNCTION__);\
	} while(0)


// this assert is probably not important, rather only in debug
#define _obvious(X) do { if (!(X)) { _erro("Assertation failed (_obvious) at " << __FILE__ << "+" << __LINE__); ::std::abort(); } } while(0)

// this assert could be helpful, maybe use in release
#define _assert(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__); ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _throw_error(X) do { if (!(X)) { _erro("Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__); ::std::abort(); }  } while(0)

#define _throw2( EXCEPT , MSG ) do { auto except_var = EXCEPT;  _erro("Going to throw exception. What:" << except_var.what() << " Info:" << MSG); throw except_var; } while(0)
#define _throw( EXCEPT ) _throw2(EXCEPT, "")

/** TODO document
throw std::logic_error("foo");
throw _except2( std::logic_error("foo") , "x=" << x << "y=" << y ) ;
throw _except( std::logic_error("foo")) ;
*/




#endif // include guard

