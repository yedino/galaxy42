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

// _dbg3 _dbg2 _dbg1 _info _note _warn _erro _mark
#define _info(X) do { ::std::cerr<<"info: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)
#define _note(X) do { ::std::cerr<<"note: " << __FILE__ << "+" << __LINE__ << " " << X << ::std::endl; } while(0)


#endif // include guard

