
#include "my_tests_lib.hpp"

#include "../stdplus/eint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>
#include <limits>

#include <tnetdbg.hpp>


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
	// this shows why we need eint lib: above code using trivian types
	// does not report runtime nor compile time error
}

TEST(eint, eint_minus_uchar) {
	unsigned char u1=250,u2=251;
	// unsigned char c{ u1-u2 };  EXPECT_NE(c,0); // good: compile-error
	EXPECT_NE(u1,u2);
}

TEST(eint, aggrinit_minus_int) {
	unsigned int a=251,b=250;
	unsigned int c{ eint::eint_minus(a,b) }; // good: no-compile-error
	int x = std::numeric_limits<int>::max() / 4 * 3;
	int y = std::numeric_limits<int>::min() / 2;
	long lmax  = std::numeric_limits<long>::max();
	long lmin  = std::numeric_limits<long>::min();
	unsigned long ulmax  = std::numeric_limits<unsigned long>::max();
	unsigned long ulmin  = std::numeric_limits<unsigned long>::min();
	// unsigned char xxx{ eint::eint_minus(a,b) }; // good: compile-error
	EXPECT_THROW( { unsigned int d{ eint::eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_THROW( { int z{ eint::eint_minus(y,x) }; EXPECT_EQ(z,y-x); } , std::range_error );
	EXPECT_THROW( { long less_then_min{ eint::eint_minus(lmin,1L) }; EXPECT_EQ(less_then_min,lmin-1); }, std::range_error );
	EXPECT_THROW( { long more_then_max{ eint::eint_minus(lmax,-1L) }; EXPECT_EQ(more_then_max,lmax+1); }, std::range_error );
	EXPECT_THROW( { unsigned long less_then_min{ eint::eint_minus(ulmin,1UL) }; EXPECT_EQ(less_then_min,ulmin-1); }, std::range_error );
	//EXPECT_THROW( { unsigned long more_then_max{ eint::eint_minus(ulmax,-1UL) }; EXPECT_EQ(more_then_max,ulmax+1); }, std::range_error );

	long result1{ eint::eint_minus(lmin, -1L) };
	long result2{ eint::eint_minus(lmax, 1L) };
	long result3{ eint::eint_minus(lmin, 0L) };
	long result4{ eint::eint_minus(lmax, 0L) };
	unsigned long result5{ eint::eint_minus(ulmax, 1UL) };
	unsigned long result6{ eint::eint_minus(ulmin, 0UL) };
	unsigned long result7{ eint::eint_minus(ulmax, 0UL) };
	EXPECT_EQ(result1, lmin + 1L);
	EXPECT_EQ(result2, lmax - 1L);
	EXPECT_EQ(result3, lmin - 0L);
	EXPECT_EQ(result4, lmax - 0L);
	EXPECT_EQ(result5, ulmax - 1UL);
	EXPECT_EQ(result6, ulmin - 0UL);
	EXPECT_EQ(result7, ulmax - 0UL);
	EXPECT_EQ(c,a-b);
	EXPECT_EQ(ULL(c) , ULL(a)-ULL(b));
}

TEST(eint, aggrinit_plus_int) {
	::g_dbg_level_set(10,"test");
	unsigned char a=251,b=250;
	int x = std::numeric_limits<int>::min() / 4 * 3;
	int y = std::numeric_limits<int>::min() / 2;
	long lmax  = std::numeric_limits<long>::max();
	long lmin  = std::numeric_limits<long>::min();
	unsigned long ulmax  = std::numeric_limits<unsigned long>::max();
	unsigned long ulmin  = std::numeric_limits<unsigned long>::min();
	EXPECT_THROW( { unsigned char d{ eint::eint_plus(b,a) }; EXPECT_EQ(d,b+a); } , std::range_error );
	EXPECT_THROW( { int z{ eint::eint_plus(y,x) }; EXPECT_EQ(z,y+x); } , std::range_error );
	EXPECT_THROW( { long less_then_min{ eint::eint_plus(lmin,-1L) }; EXPECT_EQ(less_then_min,lmin-1); }, std::range_error );
	EXPECT_THROW( { long more_then_max{ eint::eint_plus(lmax,1L) }; EXPECT_EQ(more_then_max,lmax+1); }, std::range_error );
	EXPECT_THROW( { unsigned long more_then_max{ eint::eint_plus(ulmax,1UL) }; EXPECT_EQ(more_then_max,ulmax+1); }, std::range_error );

	long result1{ eint::eint_plus(lmin, 1L) };
	long result2{ eint::eint_plus(lmax, -1L) };
	long result3{ eint::eint_plus(lmin, 0L) };
	long result4{ eint::eint_plus(lmax, 0L) };
	unsigned long result5{ eint::eint_plus(ulmin, 1UL) };
	unsigned long result6{ eint::eint_plus(ulmin, 0UL) };
	unsigned long result7{ eint::eint_plus(ulmax, 0UL) };
	EXPECT_EQ(result1, lmin + 1L);
	EXPECT_EQ(result2, lmax - 1L);
	EXPECT_EQ(result3, lmin - 0L);
	EXPECT_EQ(result4, lmax - 0L);
	EXPECT_EQ(result5, ulmin + 1UL);
	EXPECT_EQ(result6, ulmin - 0UL);
	EXPECT_EQ(result7, ulmax - 0UL);
}

TEST(eint, aggrinit_size_t) {
	size_t a=251,b=250;
	size_t c{ eint::eint_minus(a,b) }; // good: no-compile-error
	// unsigned int xxx{ eint::eint_minus(a,b) }; // good: compile-error
	EXPECT_THROW( { size_t d{ eint::eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_EQ(c,a-b);
	EXPECT_EQ(ULL(c) , ULL(a)-ULL(b));
}
