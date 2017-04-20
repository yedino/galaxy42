// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

/**
@file stdplus is library of very common functions to be used in this and other projects,
and stdplus/misc are assorted parts of that
@owner rfree
@todo TODO@testcase - test cases for all functions here
*/

#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <limits>
#include <boost/numeric/conversion/cast.hpp>

#include <limits>

#include "tnetdbg.hpp"
#include "utils/check.hpp"

namespace stdplus {

// ===========================================================================================================
// -------------------------------------------------------------------
// casting and conversions

template <typename T>
std::string STR(const T & obj) { ///< stringify any object
	std::ostringstream oss;
	oss << obj;
	return oss.str();
}

template <typename TE>
TE int_to_enum(int i) { ///< convert integer into enum of given type, throw if this value is not represented in target enum
	// TODO assert that the type TE is an defined enum here?
	static_assert( std::is_enum<TE>::value , "Must be an enum type (defined enum)");

	// assert that the type i will over/under flow (e.g. wrap back to some valid value causing undetected error)
	_check_input( static_cast<long long int>(i)  <= static_cast<long long int>( std::numeric_limits< typename std::underlying_type<TE>::type >::max() ) ); // TODO it's afe compare - upcast both types to long long... make tool for this idiom?
	_check_input( i >= 0 );

	TE e = static_cast<TE>(i);
	if (! enum_is_valid_value(e) ) {
		std::string err = "Can not convert integer " + std::to_string(i)
		+ std::string(" to enum of type ") + std::string(typeid(TE).name())
		;
		throw std::runtime_error(err);
	}
	return e;
}

std::string to_string(const std::string & v); ///< just to have identical syntax

// ===========================================================================================================
// memory casting

template <typename T, typename U>
T& unique_cast_ref(std::unique_ptr<U> & u) {
	return dynamic_cast<T&>( * u.get() );
}

template <typename T, typename U>
T* unique_cast_ptr(std::unique_ptr<U> & u) {
	return dynamic_cast<T*>( u.get() );
}

// ===========================================================================================================
// exception types

/// For exceptions that are not errors, but are expected
/// alternative situation in program.
class expected_exception : public std::exception {
	public:
		const char* what() const noexcept override;
};

/// Throw this if some element was not found, but this is normal situation.
/// E.g. when public-key was not yet created at all (to differentiate
/// from case where public-key was found but can not be loaded due to format errors etc)
class expected_not_found : public stdplus::expected_exception {
	public:
		const char* what() const noexcept override;
};


template<typename T> constexpr bool templated_always_false() { return false; }


// === ranges, copy ===

/**
 * @brief Checks if the range [start1,end1) overlaps (intersects) with range [start2,end2),
 * name "oc" is for Open,Closed range, name "ne" is for Not-Empty,
 * and asserted means that we assert validity of input data.
 * @note It assumes that ranges are properly defined end not-empty, that is: start1<end1 && start2<end2
 */

#if defined (__MINGW32__)
	#undef _assert
#endif
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
	_UNUSED(start1);
	_UNUSED(end1);
	_UNUSED(start2);
	_UNUSED(end2);
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




namespace tunserver_utils {

std::pair< std::string,int > parse_ip_string(const std::string& ip_string);

}

