#ifndef PLIB_NEW_TEST_SUITE_MACRO_HPP
#define PLIB_NEW_TEST_SUITE_MACRO_HPP
#include "general_macro_overloading.hpp"

#define new_test_suite(...) VFUNC(new_test_suite, __VA_ARGS__)

#define new_test_suite1(suite_name) ptest::ptest_suite suite_name(#suite_name)
#define new_test_suite2(var_name,suite_name) ptest::ptest_suite var_name(suite_name)

#endif //PLIB_NEW_TEST_SUITE_MACRO_HPP
