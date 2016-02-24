#ifndef PLIB_PTEST_ASSERT_MACRO_HPP
#define PLIB_PTEST_ASSERT_MACRO_HPP
#include "general_macro_overloading.hpp"

#define run_assert(...) VFUNC(run_assert, __VA_ARGS__)

#define run_assert1(expr) (\
ptest::general_suite.run_assertion(expr,#expr), expr\
)
#define run_assert2(expr,msg) (\
ptest::general_suite.run_assertion(expr,#expr,msg), expr\
)

#define run_suite_assert(...) VFUNC(run_suite_assert, __VA_ARGS__)

#define run_suite_assert2(suite,expr) (\
suite.run_assertion(expr,#expr), expr\
)
#define run_suite_assert3(suite,expr,msg) (\
suite.run_assertion(expr,#expr,msg), expr\
)

#define ppassed true
#define pfailed false

#endif //PLIB_PTEST_ASSERT_MACRO_HPP
