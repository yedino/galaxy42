#pragma once
#include <sstream>
#include <stdexcept>
#include <typeinfo>
#include <limits>

namespace stdplus {

/**
 * @namespace eint - exception int - function for safe integer operationsm that will throw exception if runtime arguments used
 * would result in mathemathically incorrect result, remember to assign the returned values in safe initializer that can not
 * narrow the argument, e.g.  unsigned int x{ eint_minus(a,b); } for compile-time check against incorrect type when
 * type of e.g. "a" is narrower then type of "x".
 * @warning remember to use aggregate initialization to avoid the narrowing as above.
 * @see http://eel.is/c++draft/dcl.init.aggr
 * @see http://en.cppreference.com/w/cpp/language/aggregate_initialization
 * @owner rfree
 */
namespace eint {

/// Calculate (a-b) in safe way (see documentation and WARNINGS about this namespace)
/// @TODO can variable "b" be silently converted to type T here, including narrowing? if yes then template on T1,T2 and
/// ...check if b can be properly expanded to T1 ?
template <typename T>
T eint_minus(T a, T b, typename std::enable_if_t<std::is_integral<T>::value>* = 0) {
	static_assert(std::numeric_limits<T>::min()<=0, "Integral type does not include 0");
	static_assert(std::numeric_limits<T>::max()>=0, "Integral type does not include 0"); // it is normal integer with 0 value
	bool is_overflow=false;
	if (b<0) {
		// Here with (b<0) the overflow can be e.g. 100 - (-30) = 130 that overflows e.g. signed char (max=127)
		// This test detects (a=100 b=-30) as: (max+b < a) -> (127 + -30 < 100) -> (97<100) -> true
		// Edge case overflow: (a=100 b=-28) as: (max+b < a) -> (127 + -28 < 100) -> (99<100) -> true
		// Edge case no-overflow: (a=100 b=-27) as: (max+b < a) -> (127 + -27 < 100) -> (100<100) -> false
		if (std::numeric_limits<T>::max()+b < a) is_overflow = true;
		// We do not have to check min because if b<0, a-b will never < min, because (-b)>=0 so a-b>=a and a>=min
	}
	else if (b>0) {
		// Here with (b>0) the overflow can be e.g. (-100) - 30 = (-130) that overflows e.g. signed char (min=(-128))
		// This test detects (a=-100 b=30) as: (min+b > a) -> (-128 + 30 > -100) -> (-98>-100) -> true
		// Edge case overflow: (a=-100 b=29) as: (min+b > a) -> (-128 + 29 > -100) -> (-99>-100) -> true
		// Edge case no-overflow: (a=-100 b=28) as: (min+b > a) -> (-128 + 28 > -100) -> (-100>-100) -> false
		if (std::numeric_limits<T>::min()+b > a) is_overflow = true;
		// We do not have to check max because if b>0, a-b will never > max, because b>0 so a-b<=a and a<=max
	}
	// If b==0 no overflow
	if (is_overflow) {
		std::ostringstream oss; oss<<"Math error when calculating (" << a << " - " << b << ") for T="<<typeid(T).name();
		throw std::range_error(oss.str());
	}
	return a-b;
}

/// Calculate (a+b) in safe way (see documentation and WARNINGS about this namespace)
/// @TODO can variable "b" be silently converted to type T here, including narrowing? if yes then template on T1,T2 and
/// ...check if b can be properly expanded to T1 ?
template <typename T>
T eint_plus(T a, T b, typename std::enable_if_t<std::is_integral<T>::value>* = 0) {
	static_assert(std::numeric_limits<T>::min()<=0, "Integral type does not include 0");
	static_assert(std::numeric_limits<T>::max()>=0, "Integral type does not include 0"); // it is normal integer with 0 value
	bool is_overflow=false;
	if (a>0) {
		// Here with (a>0) the overflow can be e.g. 100 + 30 = 130 that overflows e.g. signed char (max=127)
		// This test detects (a=100 b=30) as: (max-a < b) -> (127 - 100 < 30) -> (27<30) -> true
		// Edge case overflow: (a=100 b=28) as: (max-a < b) -> (127 - 100 < 28) -> (27<28) -> true
		// Edge case no-overflow: (a=100 b=27) as: (max-a < b) -> (127 - 100 < 27) -> (27<27) -> false
		if (std::numeric_limits<T>::max()-a < b) is_overflow = true;
		// We do not have to check min because if a>0, a+b will never < min, because a>0 so a+b>=b and b>=min
	}
	else if (a<0) {
		// Here with (a<0) the overflow can be e.g. (-100) + (-30) = (-130) that overflows e.g. signed char (min=128)
		// This test detects (a=-100 b=-30) as: (min-a > b) -> (-128 - (-100) > -30) -> (-28>-30) -> true
		// Edge case overflow: (a=-100 b=-29) as: (min-a > b) -> (-128 - -(-100) > -29) -> (-28>-29) -> true
		// Edge case no-overflow: (a=-100 b=-28) as: (min-a > b) -> (-128 - (-100) > -28) -> (-28>-28) -> false
		if (std::numeric_limits<T>::min()-a > b) is_overflow = true;
		// We do not have to check max because if a<0, a+b will never > max, because a<0 so a+b<=b and b<=max
	}
	// If b==0 no overflow
	if (is_overflow) {
		std::ostringstream oss; oss<<"Math error when calculating (" << a << " + " << b << ") for T="<<typeid(T).name();
		throw std::range_error(oss.str());
	}
	return a+b;
}

/// Compare (a<b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_less(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_unsigned<Ta>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Tb>::value>* = 0) {
	if (b<0) return false; // if b<0 and a is unsigned then always (a>b)
	else {
		if (std::numeric_limits<Ta>::digits >= std::numeric_limits<Tb>::digits){
			return a < static_cast<Ta>(b);
			// if (b>=0 and type Tb don't have more bits than type Ta) then we can safely cast b to type Ta
		}
		else return static_cast<Tb>(a) < b;
		// if (type Tb have more bits than type Ta) then we can safely cast a to type Tb
	}
}

/// Compare (a<b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_less(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Ta>::value>* = 0
							, typename std::enable_if_t<std::is_unsigned<Tb>::value>* = 0) {
	if (a<0) return true; // if a<0 and b is unsigned then always (a<b)
	else {
		if (std::numeric_limits<Tb>::digits >= std::numeric_limits<Ta>::digits){
			return static_cast<Tb>(a) < b;
			// if (a>=0 and type Ta don't have more bits than type Tb) then we can safely cast a to type Tb
		}
		else return a < static_cast<Ta>(b);
		// if (type Ta have more bits than type Ta) then we can safely cast b to type Ta
	}
}

/// Compare (a<b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_less(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Ta>::value == std::is_signed<Tb>::value>* = 0) {
	return a<b; // if both type Ta and Tb are signed or unsigned we can safely compare a and b
}

/// Compare (a==b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_equal(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_unsigned<Ta>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Tb>::value>* = 0) {
	if (b<0) return false; // if b<0 and a is unsigned then always (a>b)
	else {
		if (std::numeric_limits<Ta>::digits >= std::numeric_limits<Tb>::digits){
			return a == static_cast<Ta>(b);
			// if (b>=0 and type Tb don't have more bits than type Ta) then we can safely cast b to type Ta
		}
		else return static_cast<Tb>(a)==b;
		// if (type Tb have more bits than type Ta) then we can safely cast a to type Tb
	}
}

/// Compare (a==b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_equal(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Ta>::value>* = 0
							, typename std::enable_if_t<std::is_unsigned<Tb>::value>* = 0) {
	if (a<0) return false; // if a<0 and b is unsigned then always (a<b)
	else {
		if (std::numeric_limits<Tb>::digits >= std::numeric_limits<Ta>::digits){
			return static_cast<Tb>(a) == b;
			// if (a>=0 and type Ta don't have more bits than type Tb) then we can safely cast a to type Tb
		}
		else return a==static_cast<Ta>(b);
		// if (type Ta have more bits than type Ta) then we can safely cast b to type Ta
	}
}

/// Compare (a==b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_equal(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0
							, typename std::enable_if_t<std::is_signed<Ta>::value == std::is_signed<Tb>::value>* = 0) {
	return a==b; // if both type Ta and Tb are signed or unsigned we can safely compare a and b
}

/// Compare (a>b) in safe way (see documentation and WARNINGS about this namespace)
template <typename Ta, typename Tb>
bool eint_greater(Ta a, Tb b, typename std::enable_if_t<std::is_integral<Ta>::value && std::is_integral<Tb>::value>* = 0) {
	return !(eint_less(a, b) || eint_equal(a, b) ); //!(a<=b)
}

} // namespace eint

} // namespace stdplus
