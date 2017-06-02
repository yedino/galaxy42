
#include "my_tests_lib.hpp"

#include "../stdplus/eint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>

namespace eint {
	using namespace stdplus::eint;
}

/// helper templates to calculate the correct expected result
template <typename T>
static signed long long int SLL(T val) { return static_cast<signed long long int>(val); }

template <typename T>
static unsigned long long int ULL(T val) { return static_cast<unsigned long long int>(val); }

TEST(eint, aggrinit_narrow_uchar) {
	unsigned char u1=250,u2=251;
	// unsigned char c{ u1-u2 };  expect_gt(c,0); // good: compile-error
	EXPECT_NE(u1,u2);
}

TEST(eint, aggrinit_narrow_init) {
	unsigned int u1=250,u2=251;
	unsigned int c{ u1-u2 }; // bad: no-compile-error
	EXPECT_NE(u1,u2);
	EXPECT_EQ(c,u1-u2);
}

TEST(eint, eint_minus_uchar) {
	unsigned char u1=250,u2=251;
	// unsigned char c{ u1-u2 };  EXPECT_NE(c,0); // good: compile-error
	EXPECT_NE(u1,u2);
}

TEST(eint, aggrinit_minus_int) {
	unsigned int a=251,b=250;
	unsigned int c{ eint::eint_minus(a,b) }; // good: no-compile-error
	// unsigned char xxx{ eint::eint_minus(a,b) }; // good: compile-error
	EXPECT_THROW( { unsigned int d{ eint::eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_EQ(c,a-b);
	EXPECT_EQ(ULL(c) , ULL(a)-ULL(b));
}

TEST(eint, aggrinit_size_t) {
	size_t a=251,b=250;
	size_t c{ eint::eint_minus(a,b) }; // good: no-compile-error
	// unsigned int xxx{ eint::eint_minus(a,b) }; // good: compile-error
	EXPECT_THROW( { size_t d{ eint::eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_EQ(c,a-b);
	EXPECT_EQ(ULL(c) , ULL(a)-ULL(b));
}

TEST(eint, safe_assign_from_minus) {
	/*size_t a=300,b=50;
	unsigned char c{ a+b };
	EXPECT_GT(d,250-1);
	EXPECT_LT(d,250+1);
	// EXPECT_EQ
	*/
}

