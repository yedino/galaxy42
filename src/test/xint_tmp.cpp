
#include "gtest/gtest.h"
#include "../xint.hpp"
#include "../libs1.hpp"
#include <exception>
#include <cmath>
#include <type_traits>


/*
TEST(xinttmp, simple) {
	try {

		float x=1000000000,y=100000000,z=999999999;
		// safer_int<int> aaa( x*y*z );
		xint bbb( x*y*z );

	} catch(...) { }
}


TEST(xint_tmp, safe_create_float1) {
	auto func = []() { float a=10000, b=10000000000, c=100000000;
		xint bonus(a*b*c); UNUSED(bonus);	} ;
	EXPECT_THROW( func()  , boost::numeric::bad_numeric_cast );
}
*/


TEST(xint_tmp, check_size_of_various_types) {
//	g_dbg_level_set(30,"Details of test");

	typedef float T;
	_dbg1(std::numeric_limits<T>::max());
	_dbg1(std::numeric_limits<T>::digits);
	_dbg1(std::numeric_limits<T>::max_exponent);

	T obj( std::numeric_limits<T>::max() );
	_info("obj as float: " << obj);

	typedef boost::multiprecision::number<
		boost::multiprecision::cpp_int_backend<
			std::numeric_limits<T>::max_exponent,
			std::numeric_limits<T>::max_exponent,
			boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void>
		>
	T_as_int;

	T_as_int as_int(obj);
	_info("obj as int: " << as_int);
	T back(as_int);
	_info("obj bask as float: " << back);
	auto diff = obj-back;
	_note("DIFF: " << diff);
	EXPECT_EQ(diff,0);
}

/*
// doing tests how to compare with float in safe way. not needed probably
TEST(xint_tmp, compare_with_float) {
	basic_xint aaa(4);
	float f(3.14);
//	cpp_bin_float_100 fff(f);
	bool bigger = aaa > f;
	EXPECT_TRUE(bigger);
}
*/
