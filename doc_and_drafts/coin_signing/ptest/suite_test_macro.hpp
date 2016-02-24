#ifndef PLIB_PTEST_SUITE_TEST_MACRO_HPP
#define PLIB_PTEST_SUITE_TEST_MACRO_HPP
#include "general_macro_overloading.hpp"

#define run_suite_test(...) VFUNC(run_suite_test, __VA_ARGS__)

#define run_suite_test4(suite,func,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{});\
}
#define run_suite_test5(suite,func,x0,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0},x0);\
}
#define run_suite_test6(suite,func,x0,x1,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1},x0,x1);\
}
#define run_suite_test7(suite,func,x0,x1,x2,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2},x0,x1,x2);\
}
#define run_suite_test8(suite,func,x0,x1,x2,x3,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3},x0,x1,x2,x3);\
}
#define run_suite_test9(suite,func,x0,x1,x2,x3,x4,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4},x0,x1,x2,x3,x4);\
}
#define run_suite_test10(suite,func,x0,x1,x2,x3,x4,x5,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5},x0,x1,x2,x3,x4,x5);\
}
#define run_suite_test11(suite,func,x0,x1,x2,x3,x4,x5,x6,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6},x0,x1,x2,x3,x4,x5,x6);\
}
#define run_suite_test12(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7},x0,x1,x2,x3,x4,x5,x6,x7);\
}
#define run_suite_test13(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8},x0,x1,x2,x3,x4,x5,x6,x7,x8);\
}
#define run_suite_test14(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9);\
}
#define run_suite_test15(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10);\
}
#define run_suite_test16(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11);\
}
#define run_suite_test17(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12);\
}
#define run_suite_test18(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13);\
}
#define run_suite_test19(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14);\
}
#define run_suite_test20(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15);\
}
#define run_suite_test21(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16);\
}
#define run_suite_test22(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17);\
}
#define run_suite_test23(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18);\
}
#define run_suite_test24(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19);\
}
#define run_suite_test25(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20);\
}
#define run_suite_test26(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21);\
}
#define run_suite_test27(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22);\
}
#define run_suite_test28(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23);\
}
#define run_suite_test29(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24);\
}
#define run_suite_test30(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25);\
}
#define run_suite_test31(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26);\
}
#define run_suite_test32(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27);\
}
#define run_suite_test33(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28);\
}
#define run_suite_test34(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29);\
}
#define run_suite_test35(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30);\
}
#define run_suite_test36(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31);\
}
#define run_suite_test37(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32);\
}
#define run_suite_test38(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33);\
}
#define run_suite_test39(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34);\
}
#define run_suite_test40(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35);\
}
#define run_suite_test41(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36);\
}
#define run_suite_test42(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37);\
}
#define run_suite_test43(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38);\
}
#define run_suite_test44(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39);\
}
#define run_suite_test45(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40);\
}
#define run_suite_test46(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41);\
}
#define run_suite_test47(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42);\
}
#define run_suite_test48(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43);\
}
#define run_suite_test49(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44);\
}
#define run_suite_test50(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45);\
}
#define run_suite_test51(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46);\
}
#define run_suite_test52(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47);\
}
#define run_suite_test53(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48);\
}
#define run_suite_test54(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49);\
}
#define run_suite_test55(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50);\
}
#define run_suite_test56(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51);\
}
#define run_suite_test57(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52);\
}
#define run_suite_test58(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53);\
}
#define run_suite_test59(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53,#x54},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54);\
}
#define run_suite_test60(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53,#x54,#x55},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55);\
}
#define run_suite_test61(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53,#x54,#x55,#x56},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56);\
}
#define run_suite_test62(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56,x57,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53,#x54,#x55,#x56,#x57},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56,x57);\
}
#define run_suite_test63(suite,func,x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56,x57,x58,expected_result,equality)\
{\
suite.start_test(func,#func,expected_result,equality,{#x0,#x1,#x2,#x3,#x4,#x5,#x6,#x7,#x8,#x9,#x10,#x11,#x12,#x13,#x14,#x15,#x16,#x17,#x18,#x19,#x20,#x21,#x22,#x23,#x24,#x25,#x26,#x27,#x28,#x29,#x30,#x31,#x32,#x33,#x34,#x35,#x36,#x37,#x38,#x39,#x40,#x41,#x42,#x43,#x44,#x45,#x46,#x47,#x48,#x49,#x50,#x51,#x52,#x53,#x54,#x55,#x56,#x57,#x58},x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15,x16,x17,x18,x19,x20,x21,x22,x23,x24,x25,x26,x27,x28,x29,x30,x31,x32,x33,x34,x35,x36,x37,x38,x39,x40,x41,x42,x43,x44,x45,x46,x47,x48,x49,x50,x51,x52,x53,x54,x55,x56,x57,x58);\
}


#endif //PLIB_PTEST_SUITE_TEST_MACRO_HPP
