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
#include <functional> // for less

#include <limits>

#include "tnetdbg.hpp" // debug
#include "strings_utils_base.hpp" // to create to_debug()
#include "utils/check.hpp"

namespace stdplus {

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


// ===========================================================================================================
// -------------------------------------------------------------------
// casting and conversions

template <typename T>
std::string STR(const T & obj) { ///< stringify any object
	std::ostringstream oss;
	oss << obj;
	return oss.str();
}

/// convert integer into enum of given type, throw if this value is not represented in target enum
/// if expected_bad==true, then for invalid enum thrown exception type is 'expected_not_found'
template <typename TE>
TE int_to_enum(int i, bool expected_bad=false) {
	static_assert( std::is_enum<TE>::value , "Must be an enum type (defined enum)");

	// assert that the type i will over/under flow (e.g. wrap back to some valid value causing undetected error)
	_check_input( static_cast<long long int>(i)  <= static_cast<long long int>( std::numeric_limits< typename std::underlying_type<TE>::type >::max() ) ); // TODO it's afe compare - upcast both types to long long... make tool for this idiom?
	_check_input( i >= 0 );

	TE e = static_cast<TE>(i);
	if (! enum_is_valid_value(e) ) {
		if (expected_bad) throw expected_not_found();
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
// === ranges, copy ===


/// AddressibleIterator
/// this is our concept of an iterator that meets all of following conditions:
/// - it's instance is a valid iterator that can be dereferenced, e.g. it is not some vector_v.end()
/// - it's type can be converted to void* and to T*

/// AddressComparableIterator
/// this is our concept of an iterator that meets all of following conditions:
/// - is an AddressibleIterator
/// - can be ordered (has correct operations < <= > >= done by std:: less less_equal greater greater_equal)
///   (though anyway we will often compare the resulting raw address void* instead)

/// @warning the iterator iter must be an AddressibleIterator
/// @return the object pointed to by given iterator, in form of void*;
template <typename T>
const void * iterator_to_voidptr(T iter) {
	typedef typename std::iterator_traits<T>::value_type value_type;
	const value_type & obj = * iter; // pointed object, as some type
	const void* addr = static_cast<const void*>(&obj); // as raw address
	return addr;
}

template <typename T>
std::string to_debug(const T & X, t_debug_style style=e_debug_style_object
	,typename std::enable_if<
		std::is_same<  typename std::iterator_traits<T>::iterator_category , std::random_access_iterator_tag	 >
	::value >::type * = 0)
{
	(void)(style); // unused
	std::ostringstream oss;
	oss << "iter("<<iterator_to_voidptr(X)<<")";
	return oss.str();
}

/// @warning the iterators iter1, iter2 must be an AddressComparableIterator
/// @return whether iter1 points to memory address lower then iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_less(T1 iter1, T2 iter2) {
	return std::less<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

/// @warning the iterators iter1, iter2 must be an AddressComparableIterator
/// @return whether iter1 points to memory address lower or equal to iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_less_equal(T1 iter1, T2 iter2) {
	bool ret = std::less_equal<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
	_dbg4("Compare <= says: " << iterator_to_voidptr(iter1) << ( ret ? " <= " : " no " ) << iterator_to_voidptr(iter2)  );
	return ret;
}

/// @warning the iterators iter1, iter2 must be an AddressComparableIterator
/// @return whether iter1 points to memory address higher then iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_greater(T1 iter1, T2 iter2) {
	return std::greater<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

/// @warning the iterators iter1, iter2 must be an AddressComparableIterator
/// @return whether iter1 points to memory address higher or equal to iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_greater_equal(T1 iter1, T2 iter2) {
	return std::greater_equal<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

/**
Tests whether range [start1..last1] overlaps with [start2..last2].
This ranges are given inclusive.
All this iterators must be valid (dereferencable).
Therefore to express entire e.g. vector<T> v, you would give range v.begin() , v.end()-1,
after first checking that v is not empty.
*/
template <typename TIn, typename TOut> bool test_ranges_overlap_notempty_asserted(TIn start1, TIn last1,  TOut start2, TOut last2) noexcept {
	_check_abort( std::less_equal<decltype(start1)>() (start1, last1) );
	_check_abort( std::less_equal<decltype(start1)>() (start2, last2) );

	// start1 < last2 && (start2 < last1)
	return ( iterator_less_equal(start1,last2) && iterator_less_equal(start2,last1) );
}

/**
 * Safe copy of memory from range [first..last] into [d_first, d_first+size-1]
 * This iterator should be of type
 *
 * It always checks following conditions:
 * - the memory ranges must not overlap
 * - they must be not-empty, and valid (begin < last)
 * - the size given as argument must match the size of range [first..last]
 * if conditions are not correct, it can throw (at least for the case of normal overlapping), or abort.
 *
 * It does not check following conditions and they must be guaranteed by caller, else UB:
 * - all given iterators must be valid (dereferencable)
 * - the d_first + size must be a valid (dereferencable) iterator
 * - the ranges defined by them must be valid too, e.g. all elements [first..last], and [d_first..d_first+size-1]
 * must be valid
 */
template <typename TIn, typename TOut> void copy_and_assert_no_overlap_size(TIn first, TIn last, TOut d_first, size_t size) {
	_dbg4("copy first="<<to_debug(first)<<" last="<<to_debug(last)<<" d_first="<<to_debug(d_first)<<" size="<<size);
	_check_abort( iterator_less_equal( first, last ) );
	_check_abort( size > 0 );
	_check_input( ! test_ranges_overlap_notempty_asserted(first, last, d_first, d_first+size-1) ); // overlap?
	std::copy(first, std::next(last), d_first); // copy. next, because std::copy takes iterator to end (not to last)
}


} // namespace




namespace tunserver_utils {

std::pair< std::string,int > parse_ip_string(const std::string& ip_string);

}

