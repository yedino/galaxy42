// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef LIBS0_HPP
#define LIBS0_HPP

/**

@file Libs0 is collection of basic commonly used library parts (global as well as local libraries),
parts that developers of this project expect to be always ready to be used, e.g. most common containers, memory, debug.
1) It included them,
2) and it makes it available in global namespace by "using N::X", and occasionally by "using namespace N".

Also, some entities (macros, functions, classes) are still defined here directly, how ever this will be eventually cleared out,
moved into e.g. src/utils/... and src/stdplus/... and this file will be purely includes+usings.
@owner rfree

*/


#include "project.hpp"

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <chrono>
#include <thread>
#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <cmath>
#include <random>
#include <algorithm>
#include <exception>
#include <mutex>
#include <cassert>
#include <array>
#include <set>
#include <tuple>
#include <iomanip>
#include <unordered_set>
#include <regex>
#include <type_traits>

#include "tnetdbg.hpp"
#include "strings_utils_simple.hpp"
#include "mo_reader.hpp"
#include "platform.hpp"

#include "utils/check.hpp"

#include <utils/unused.hpp>

#include "stdplus/misc.hpp"
#include "mutex.hpp" // clang thread safety analysis
#include "stdplus/eint.hpp"

using std::string;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;

using std::make_shared;

using std::vector;
using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::unordered_set;

using std::ostream;
using std::ifstream;
using std::istringstream;
using std::ostringstream;

using std::endl;

using boost::numeric_cast;

using std::runtime_error;
using std::invalid_argument;

using std::make_unique;

using stdplus::eint::eint_minus;

using namespace std::string_literals; // <=== using entire namespace


using namespace stdplus;

#define SVAR(x) #x << " = " << x


// === RELEASE OPTIONS === (release/debug mode flags)
#if defined (RELEASEMODE_)
	#define OPTION_DEBUG_SHOW_SECRET_STRINGS 0
#else
	#define OPTION_DEBUG_SHOW_SECRET_STRINGS 1
#endif

#if OPTION_DEBUG_SHOW_SECRET_STRINGS
	#define DEBUG_SECRET_STR( X ) ( X )
#else
	#define DEBUG_SECRET_STR( X ) ( "[hidden-secret]" )
#endif

// PRETTY FUNCTION NAME - DEBUG
#ifndef __PRETTY_FUNCTION__
	#ifdef WIN32   //WINDOWS
	#define __PRETTY_FUNCTION__   __FUNCTION__
#else          //*NIX
	#define __PRETTY_FUNCTION__   __func__
	#endif
#endif

// ==================================================================

// --- TODO https://h.mantis.antinet.org/view.php?id=37 ---
/***
 * type that is a size_t, but can be with error-signaling value e.g. -1, otherwise it is valid.
 * the caller MUST check the returned value against size_t_is() before using it
 */
typedef size_t size_t_maybe;

inline size_t_maybe size_t_invalid() { ///< returns a size_t that means "invalid size_t"
	return static_cast<size_t>( -1 );
}

/***
 * returns if given size_t is correct, or is it the invalid value
*/
inline bool size_t_is_ok(size_t x) {
	if (x == size_t_invalid()) return false;
	return true;
}
// -----------------------------------------


/*
template <typename T> const T & PTR_assert(const T & ptr, const char *func) {
	if (!(ptr!=nullptr)) {
		_erro("NULL pointer used! from func="<<func);
        throw std::invalid_argument("Null pointer");
        //std::abort();
		//assert(ptr!=nullptr);
	}
	return ptr;
}
*/


#if defined(_MSC_VER)
const char * gettext(const char * s);
char * bindtextdomain(const char * domainname, const char * dirname);
char * textdomain(const char * domainname);
#endif



#endif
