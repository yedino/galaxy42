#pragma once
#ifndef LIBS1_HPP
#define LIBS1_HPP

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

#include "c_tnetdbg.hpp"

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

using std::endl;

#define UNUSED(expr) do { (void)(expr); } while (0)


// === RELEASE OPTIONS ===
#ifndef RELEASE
	#define RELEASE 0
#endif

#ifndef DEBUG_SHOW_SECRET_STRINGS
	#if RELEASE
		#define DEBUG_SHOW_SECRET_STRINGS 0
	#else
		#define DEBUG_SHOW_SECRET_STRINGS 1
	#endif
#endif

#if DEBUG_SHOW_SECRET_STRINGS
	#define DEBUG_SECRET_STR( X ) ( X )
#else
	#define DEBUG_SECRET_STR( X ) ( "(a secret string, hidden by compilation options)" )
#endif

// ??? decide: XXX
#include "c_tnetdbg.hpp"

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

// extending the std with helpfull tools
namespace std {


// this is due to enter C++14
// http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
template <typename T, typename... Args>
std::unique_ptr<T> make_unique (Args &&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

using std::make_unique;


// extending the std with helpfull tools - by own idea
namespace stdplus {

template <typename T, typename U>
T& unique_cast_ref(std::unique_ptr<U> & u) {
	return dynamic_cast<T&>( * u.get() );
}

template <typename T, typename U>
T* unique_cast_ptr(std::unique_ptr<U> & u) {
	return dynamic_cast<T*>( u.get() );
}

template <typename T>
std::string STR(const T & obj) {
	std::ostringstream oss;
	oss << obj;
	return oss.str();
}


class expected_exception : public std::exception {
	public:
		const char* what() const noexcept override;
};

class expected_not_found : public stdplus::expected_exception {
	public:
		const char* what() const noexcept override;
};


template <typename TE, typename TI>
TE int_to_enum(TI i) {
	TE e = static_cast<TE>(i);
	TI i_check = static_cast<TI>(e);
	if (i_check != i) {
		std::string err = "Can not convert integer " + std::to_string(i)
		+ std::string(" to enum of type ") + std::string(typeid(TE).name())
		+ std::string(" because it is truncated to other value: ") + std::to_string( i_check )
		;
		throw std::runtime_error(err);
	}
	return e;
}

std::string to_string(const std::string & v); ///< just to have identical syntax


}

using namespace stdplus;

namespace tunserver_utils {

std::pair< std::string,int > parse_ip_string(const string& ip_string);

}

#endif

