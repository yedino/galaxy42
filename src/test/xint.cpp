#include "gtest/gtest.h"
#include "../xint.hpp"
#include "../libs1.hpp"
#include <exception>
#include <cmath>

//		ASSERT_EQ(parser.pop_integer_uvarint(), i);
//	ASSERT_THROW(parser.pop_varstring(), std::exception);
//	EXPECT_NO_THROW(test::test_trivialserialize());

namespace test_xint {
namespace detail {


typedef long double t_correct1;

template<typename T_INT>
void math_tests_noproblem() {
	//_mark("Doing tests, on integral with sizeof="<<sizeof(T_INT));

	vector<int> testsize_tab = { 1, 2, 50, 1000, 10000 , 1000000 };
	for (auto testsize : testsize_tab) {
		T_INT a=0;
		int n=testsize;
		for (int i=1; i<n; ++i) a = a + i;

		t_correct1 n_co = n;
		t_correct1 a_co = (((n_co-1)*1)/2) * n_co;
		EXPECT_EQ(a, a_co);
		T_INT a_co_round = std::round(a_co);
		EXPECT_EQ(a, a_co_round);
	}
}

template <typename T_INT> bool is_safe_int() {
	bool safetype =
		(typeid(T_INT) == typeid(xint))
		|| (typeid(T_INT) == typeid(uxint))
	;
	return safetype;
}

template<typename T_INT>
void math_tests_overflow(T_INT val) { bool safetype = is_safe_int<T_INT>();
	T_INT a = val;	t_correct1 a_co = a;
	EXPECT_NO_THROW( { a++;  a_co+=1; } );  EXPECT_EQ(a,a_co); // this should fit for given starting val
	// next icrement is problematic:
	if (safetype) {
		EXPECT_THROW( { a++;	a_co+=1; } , std::runtime_error );
	} else { // should not fit for given val
		EXPECT_NO_THROW( { a++;	a_co+=1; } ); // usafe type fils to throw
		EXPECT_NE(a,a_co); // unsafe type has mathematically-invalid value
	}
}

} // namespace
} // namespace


TEST(xint, math1) {
	test_xint::detail::math_tests_noproblem<long int>();
	test_xint::detail::math_tests_noproblem<unsigned long int>();
	test_xint::detail::math_tests_noproblem<xint>();
}

TEST(xint, overflow_basic) {	test_xint::detail::math_tests_overflow<uint32_t>(0xFFFFFFFE); }
TEST(xint, overflow_xint) {	test_xint::detail::math_tests_overflow<xint>(0xFFFFFFFE); }
TEST(xint, overflow_basic_unsign) {	test_xint::detail::math_tests_overflow<int32_t>(0xFFFFFFFF/2-1); }
TEST(xint, overflow_xint_unsign) {	test_xint::detail::math_tests_overflow<uxint>(0xFFFFFFFF/2-1); }

