// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef LIBS0_HPP
#define LIBS0_HPP

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

#include <boost/numeric/conversion/cast.hpp>

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
using std::ostringstream;

using std::endl;

using boost::numeric_cast;

#define UNUSED(expr) do { (void)(expr); } while (0)
#define SVAR(x) #x << " = " << x



// === RELEASE OPTIONS ===
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


/*
// this is due to enter C++14
// http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
template <typename T, typename... Args>
std::unique_ptr<T> make_unique (Args &&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
*/

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

template<typename T> constexpr bool templated_always_false() { return false; }


// === ranges, copy ===

/**
 * @brief Checks if the range [start1,end1) overlaps (intersects) with range [start2,end2),
 * name "oc" is for Open,Closed range, name "ne" is for Not-Empty,
 * and asserted means that we assert validity of input data.
 * @note It assumes that ranges are properly defined end not-empty, that is: start1<end1 && start2<end2
 */
 #undef _assert
template <typename TIn, typename TOut> bool ranges_overlap_oc_ne_asserted(TIn start1, TIn end1,  TOut start2, TOut end2) noexcept {
	assert( (start1<end1) );
	assert( (start2<end2) );
	return ( start1 < end2 && start2 < end1 );
}

/**
 * @brief This asserts that range [start1,end1) does not overlap with range [start2,end2).
 * @note It asserts also that the given ranges are not-empty and valid (start<end).
 */
template <typename TIn, typename TOut> void assert_not_ranges_overlap_oc_ne(TIn start1, TIn end1,  TOut start2, TOut end2) noexcept {
	assert( ! ranges_overlap_oc_ne_asserted(start1,end1, start2,end2) );
}

/**
 * Very safe copy of memory from range [first..last) into [d_first, d_first+size)
 * It asserts following conditions:
 * - the memory ranges must not overlap
 * - they must be not-empty and valid (begin < end)
 * - the size given as argument must match the size of range [first..last)
 */
template <typename TIn, typename TOut> void copy_and_assert_no_overlap_size(TIn first, TIn last, TOut d_first, size_t size) {
	assert( boost::numeric_cast<size_t>(last-first) == size); // is size as expected
	// check if the memory ranges do not overlap by any chance:
	assert_not_ranges_overlap_oc_ne(first,last, d_first,d_first+size);
	std::copy(first,last, d_first); // ***
}


} // namespace


using namespace stdplus;


namespace tunserver_utils {

std::pair< std::string,int > parse_ip_string(const string& ip_string);

}


using namespace std::string_literals;

#define PTR(X) (PTR_assert(X, __func__))
template <typename T> const T & PTR_assert(const T & ptr,const char *func) {
	if (!(ptr!=nullptr)) {
		_erro("NULL pointer used! from func="<<func);
		std::abort();
		//assert(ptr!=nullptr);
	}
	return ptr;
}


#define TODOCODE { std::stringstream oss; oss<<"Trying to use a not implemented/TODO code, in " \
<<__func__<<" (line "<<__LINE__<<")"; \
_erro(oss.str()); \
throw std::runtime_error(oss.str()); \
} while(0)

#endif

