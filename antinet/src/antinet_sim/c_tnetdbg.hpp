#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP

#include <cstdlib>

// this assert is probably not important, rather only in debug
#define _obvious(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_obvious) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); } } while(0)

// this assert could be helpful, maybe use in release
#define _assert(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); }  } while(0)


/// This macros will be moved later to glorious-cpp library or other

#define _dbg3(X) do{}while(0)
// do { ::std::cerr<<"dbg3: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)

#define _dbg2(X) do { ::std::cerr<<"dbg2: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _dbg1(X) do { ::std::cerr<<"dbg1: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _info(X) do { ::std::cerr<<"info: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _note(X) do { ::std::cerr<<"note: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _warn(X) do { ::std::cerr<<"Warn! " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _erro(X) do { ::std::cerr<<"ERRO! " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _mark(X) do { \
	::std::cerr<<"\n\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	::std::cerr<<"MARK* " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; \
	::std::cerr<<"\n\n\n"; for (int i=0; i<70; ++i) ::std::cerr<<'='; ::std::cerr<<::std::endl; \
	} while(0)

#endif // include guard

