
#include "gtest/gtest.h"
#include "../xint.hpp"
#include "../libs1.hpp"
#include <exception>
#include <cmath>
#include <type_traits>

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
	func();
//	EXPECT_THROW( func()  , boost::numeric::bad_numeric_cast );
}

