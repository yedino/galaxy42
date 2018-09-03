
#include "my_tests_lib.hpp"

#include "../xint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>
#include <utils/unused.hpp>

#if USE_BOOST_MULTIPRECISION


// ===========================================================================================================
// tests v1

TEST(xint_tmp, safe_create_float1) {
	// g_dbg_level_set(30,"Details of test");

	auto func = []() { float a=10000, b=10000000000, c=100000000;
		xint bonus(a*b*c); pfp_UNUSED(bonus);	} ;
	EXPECT_THROW( func()  , boost::numeric::bad_numeric_cast );
}


TEST(xint_tmp, check_size_of_various_types) {
	typedef float T;
	pfp_dbg1(std::numeric_limits<T>::max());
	pfp_dbg1(std::numeric_limits<T>::digits);
	pfp_dbg1(std::numeric_limits<T>::max_exponent);

	T obj( std::numeric_limits<T>::max() );
	pfp_info("obj as float: " << obj);

	typedef boost::multiprecision::number<
		boost::multiprecision::cpp_int_backend<
			std::numeric_limits<T>::max_exponent,
			std::numeric_limits<T>::max_exponent,
			boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void>
		>
	T_as_int;

	T_as_int as_int(obj);
	pfp_info("obj as int: " << as_int);
	T back(as_int);
	pfp_info("obj bask as float: " << back);
	auto diff = obj-back;
	pfp_note("DIFF: " << diff);
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


// tests v1
// ===========================================================================================================

#else

#warning "USE_BOOST_MULTIPRECISION is disabled - so we can not test this part of code"

TEST(xint_tmp, disabled) {
	TEST_IS_DISABLED("Compilation options disabled USE_BOOST_MULTIPRECISION");
}

#endif

