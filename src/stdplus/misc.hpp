

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
#include <tuple>
#include <boost/numeric/conversion/cast.hpp> // numeric_cast
#include <functional> // for less

#include <limits>

#include <tnetdbg.hpp> // debug
#include <strings_utils_base.hpp> // to create to_debug()
#include <utils/check.hpp>
#include <utils/unused.hpp>

namespace stdplus {


// ===========================================================================================================

/// Gets the back() of a container, but if it's empty then throws instead doing UB
template<typename T> typename T::reference at_back(T & container) // when the container is not-const object, the we will get not-const reference
{
	if (container.empty()) throw std::runtime_error("Trying to get back() of empty container");
	return container.back();
}

/// Gets the back() of a container, but if it's empty then throws instead doing UB
template<typename T> typename T::const_reference at_back(const T & container) // when the container is const object, the we will get const reference
{
	if (container.empty()) throw std::runtime_error("Trying to get back() of empty container");
	return container.back();
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

/**
 * idiom_error_on_instantize - this is when we want to do an static_assert,
 * that triggers when templated function is instantized,
 * and not just when it's body is parsed.
 * it can be useful for idiom_error_on_missing_overload_specialization
 */

/**
 * idiom_error_on_missing_overload_specialization - this is a situation where
 * we want to show a NICE compilation error, when ever user tries to do something
 * that requires user to provide specialization/overload of function foo,
 * when user fails to provide it.
 *
 * 1) It could be done with idiom_error_on_instantize e.g. with templated_always_false()
 * 2) In future even better way of doing that can be C++20, or BoostConcepts (if we decide to use that lib)
 * 3) It can be easily done using idiom_missing_overload_as_template_false as following:
 * template <typename T, typename FALSE = std::false_type>
 * void foo(T obj) { static_assert(FALSE::value, "my error message"); }
 *
 * Method 3 would be preferred, though it does not work on MSVC (2014).
 * choosing method 1 for now.
*/

/**
 * This template returns false when a template using it is instantized. Useful for the idiom_error_on_instantize
 * @warning check whether we prefer this method of idiom_missing_overload_as_template_false or something else.
*/
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

/**
 * convert integer into enum of given type, throw if this value is not represented in target enum
 * if expected_bad==true, then for invalid enum thrown exception type is 'expected_not_found'
 */
template <typename TE, typename TI>
TE int_to_enum(TI val_int, bool expected_bad=false, typename std::enable_if<std::is_integral<TI>::value>::type* = 0)
{
	using namespace std::string_literals;

	static_assert( std::is_enum<TE>::value , "Must be an enum type (defined enum)");

	using underlying_type = typename std::underlying_type<TE>::type;

	// on conversion error, throw exception of given type from ex_type::value_type, and add additional info #why
	// TODO pass here const auto & to ex_type
	auto handle_error = [&](const std::string & why, auto ex_type) {
		if (expected_bad) throw expected_not_found();
		std::string msg = "Can not convert integer value "s + STR(val_int) + " ("s + typeid(TI).name() + ")"s
			+ " to enum of type "s + typeid(TE).name() + " "s
			+ ": "s + why + "."s
			+ " (and you wanted strict matching of enum type to be hard exception)"s;
		using t_ex = typename decltype(ex_type)::value_type;
		pfp_dbg5("Exception type to throw is: " << typeid(t_ex).name());
		t_ex ex(msg);
		pfp_throw_error(ex);
	};

	try {
		auto val_underlying = boost::numeric_cast<underlying_type>( val_int );
		TE the_enum = static_cast<TE>(val_underlying); // <--- this must succeed (without under/overflow) since it's already underlying
		if (! enum_is_valid_value(the_enum) ) handle_error("No such value defined in enum", std::vector<std::invalid_argument>() );
		return the_enum; // <=== return
	} catch(const boost::numeric::bad_numeric_cast &) {
		handle_error("Given integer value can not be expressed in enum underlying type "s
			+ " ("s + typeid(underlying_type).name()+")", std::vector<std::overflow_error>() );
	}

	pfp_DEAD_RETURN();
}

/**
 * Converts enum to the underlying type
*/
template <typename TE>
typename std::underlying_type<TE>::type enum_to_int(TE val_enum) {
	using t_int = typename std::underlying_type<TE>::type;
	return static_cast<t_int>(val_enum);
}

/**
 * Converts enum to the underlying type, while caller state what type he expects to get
 * to e.g. avoid unexpected narrowing conversion
*/
template <typename TI, typename TE>
typename std::underlying_type<TE>::type enum_to_int_safe(TE val_enum) {
	using t_int = typename std::underlying_type<TE>::type;
	static_assert( std::is_same<t_int, TI>::value , "The actuall underlying_type is not matching the requested type");
	t_int val_int{ static_cast<t_int>(val_enum) };
	return val_int;
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

/// AddressLinearIterator
/// this is our concept of an iterator that meets all of following conditions:
/// - is an AddressibleIterator
/// - can be ordered (has correct operations < <= > >= done by std:: less less_equal greater greater_equal)
///   (though anyway we will often compare the resulting raw address void* instead)

/// AddressLinearContainer - container that has it's .begin() .end() and other iterators of type AddressLinearIterator,
/// that is: std::vector<>, std::array<>, std::basic_string<>

/// @return the object pointed to by given iterator, in form of void*;
/// @warning the iterator iter must be an AddressibleIterator
template <typename T>
const void * iterator_to_voidptr(T iter
	,typename std::enable_if< // Concept
		std::is_same<  typename std::iterator_traits<T>::iterator_category , std::random_access_iterator_tag	 >
	::value >::type * = 0)
{
	using value_type = typename std::iterator_traits<T>::value_type;
	const value_type & obj = * iter; // pointed object, as some type
	const void* addr = static_cast<const void*>(&obj); // as raw address
	return addr;
}

/// @return string representing debug information about given iterator
/// @warning the iterator iter must be an AddressibleIterator
template <typename T>
std::string to_debug(const T & X, t_debug_style style=e_debug_style_object
	,typename std::enable_if< // Concept, needed to choose from other functions with same name
		std::is_same<  typename std::iterator_traits<T>::iterator_category , std::random_access_iterator_tag	 >
	::value >::type * = 0)
{
	pfp_UNUSED(style);
	std::ostringstream oss;
	oss << "iter("<<iterator_to_voidptr(X)<<")";
	return oss.str();
}

// -------------------------------------------------------------------
// Comparing iterators

/// @warning the iterators iter1, iter2 must be an AddressLinearIterator
/// @return whether iter1 points to memory address lower then iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_less(T1 iter1, T2 iter2) {
	return std::less<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

/// @warning the iterators iter1, iter2 must be an AddressLinearIterator
/// @return whether iter1 points to memory address lower or equal to iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_less_equal(T1 iter1, T2 iter2) {
	bool ret = std::less_equal<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
	pfp_dbg5("Compare<= says: " << iterator_to_voidptr(iter1) << ( ret ? "  IS<= " : " NOT<= " ) << iterator_to_voidptr(iter2)  );
	return ret;
}

/// @warning the iterators iter1, iter2 must be an AddressLinearIterator
/// @return whether iter1 points to memory address higher then iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_greater(T1 iter1, T2 iter2) {
	return std::greater<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

/// @warning the iterators iter1, iter2 must be an AddressLinearIterator
/// @return whether iter1 points to memory address higher or equal to iter2 (as by comparing void*)
template <typename T1, typename T2>
bool iterator_greater_equal(T1 iter1, T2 iter2) {
	return std::greater_equal<const void*>()( iterator_to_voidptr(iter1), iterator_to_voidptr(iter2) );
}

// -------------------------------------------------------------------
// ranges

/**
Tests whether range [start1..last1] overlaps with [start2..last2].
This ranges are given inclusive.
Therefore container/range must be NOT EMPTY (this is reminded by function name _noempty).
All this iterators must be of type AddressLinearIterator.
All this iterators must be valid (dereferencable).
Therefore to express entire e.g. vector<T> v, you would give range
as v.begin() , v.end()-1 and after first checking that v is not empty.
*/
template <typename TIn, typename TOut> bool test_ranges_overlap_inclusive_noempty(TIn start1, TIn last1,  TOut start2, TOut last2) noexcept {
	_check_abort( iterator_less_equal(start1, last1) );
	_check_abort( iterator_less_equal(start2, last2) );

	// start1 < last2 && (start2 < last1)
	return ( iterator_less_equal(start1,last2) && iterator_less_equal(start2,last1) );
}

/**
 * Safe copy of memory from range [first..last] into [d_first, d_first+size-1]
 * This iterators should be of type AddressLinearIterator
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
template <typename TIn, typename TOut> void copy_iter_and_check_no_overlap(TIn first, TIn last, TOut d_first, size_t size) {
	pfp_dbg5("copy first="<<to_debug(first)<<" last="<<to_debug(last)<<" d_first="<<to_debug(d_first)<<" size="<<size);
	_check_abort( iterator_less_equal( first, last ) );
	_check_abort( size > 0 );
	_check_input( ! test_ranges_overlap_inclusive_noempty(first, last, d_first, d_first+size-1) ); // overlap?
	std::copy(first, std::next(last), d_first); // copy. next, because std::copy takes iterator to end (not to last)
}


} // namespace




namespace tunserver_utils {

std::pair< std::string,int > parse_ip_string(const std::string& ip_string);

}

