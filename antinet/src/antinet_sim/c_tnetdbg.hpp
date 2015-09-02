#ifndef C_TNETDBG_HPP
#define C_TNETDBG_HPP

#include <cstdlib>

// this assert is probably not important, rather only in debug
#define _obvious(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_obvious) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); } } while(0)

// this assert could be helpful, maybe use in release
#define _assert(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); }  } while(0)

// this assert MUST BE checked in release too
#define _check(X) do { if (!(X)) { ::std::cerr<<"Assertation failed (_assert) at " << __FILE__ << "+" << __LINE__ << ::std::endl; ::std::abort(); }  } while(0)


#endif // include guard

