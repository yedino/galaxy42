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
bool math_tests_noproblem() {
	_mark("Doing tests, on integral with sizeof="<<sizeof(T_INT));

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
	return true;
}

} // namespace
} // namespace


TEST(xint, math1) {
	test_xint::detail::math_tests_noproblem<long int>();
	test_xint::detail::math_tests_noproblem<xint>();
}

