
#include "my_tests_lib.hpp"

#include "../stdplus/eint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>
#include <limits>
#include <xint.hpp>
#include <tnetdbg.hpp>


using namespace stdplus::eint;


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

/*
TEST(eint, aggrinit_narrow_init) {
	unsigned int u1=250,u2=251;
	// ub sanitizer shows error here so it is commented out.
	// unsigned int c{ u1-u2 }; // bad: no-compile-error
	// volatile auto unused = c;
	// this shows why we need eint lib: above code using trivian types
	// does not report runtime nor compile time error
}
*/

TEST(eint, eint_minus_uchar) {
	unsigned char u1=250,u2=251;
	// unsigned char c{ u1-u2 };  EXPECT_NE(c,0); // good: compile-error
	EXPECT_NE(u1,u2);
}

TEST(eint, aggrinit_minus_int) {
	unsigned int a=251,b=250;
	unsigned int c{ eint_minus(a,b) }; // good: no-compile-error
	int x = std::numeric_limits<int>::max() / 4 * 3;
	int y = std::numeric_limits<int>::min() / 2;
	long lmax  = std::numeric_limits<long>::max();
	long lmin  = std::numeric_limits<long>::min();
	unsigned long ulmax  = std::numeric_limits<unsigned long>::max();
	unsigned long ulmin  = std::numeric_limits<unsigned long>::min();
	EXPECT_THROW( { unsigned int d{ eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_THROW( { int z{ eint_minus(y,x) }; EXPECT_EQ(z,y-x); } , std::range_error );
	EXPECT_THROW( { long less_then_min{ eint_minus(lmin,1L) }; EXPECT_EQ(less_then_min,lmin-1); }, std::range_error );
	EXPECT_THROW( { long more_then_max{ eint_minus(lmax,-1L) }; EXPECT_EQ(more_then_max,lmax+1); }, std::range_error );
	EXPECT_THROW( { unsigned long less_then_min{ eint_minus(ulmin,1UL) }; EXPECT_EQ(less_then_min,ulmin-1); }, std::range_error );

	long result1{ eint_minus(lmin, -1L) };
	long result2{ eint_minus(lmax, 1L) };
	long result3{ eint_minus(lmin, 0L) };
	long result4{ eint_minus(lmax, 0L) };
	unsigned long result5{ eint_minus(ulmax, 1UL) };
	unsigned long result6{ eint_minus(ulmin, 0UL) };
	unsigned long result7{ eint_minus(ulmax, 0UL) };
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
	unsigned char a=251,b=250;
	int x = std::numeric_limits<int>::min() / 4 * 3;
	int y = std::numeric_limits<int>::min() / 2;
	long lmax  = std::numeric_limits<long>::max();
	long lmin  = std::numeric_limits<long>::min();
	unsigned long ulmax  = std::numeric_limits<unsigned long>::max();
	unsigned long ulmin  = std::numeric_limits<unsigned long>::min();
	EXPECT_THROW( { unsigned char d{ eint_plus(b,a) }; EXPECT_EQ(d,b+a); } , std::range_error );
	EXPECT_THROW( { int z{ eint_plus(y,x) }; EXPECT_EQ(z,y+x); } , std::range_error );
	EXPECT_THROW( { long less_then_min{ eint_plus(lmin,-1L) }; EXPECT_EQ(less_then_min,lmin-1); }, std::range_error );
	EXPECT_THROW( { long more_then_max{ eint_plus(lmax,1L) }; EXPECT_EQ(more_then_max,lmax+1); }, std::range_error );
	EXPECT_THROW( { unsigned long more_then_max{ eint_plus(ulmax,1UL) }; EXPECT_EQ(more_then_max,ulmax+1); }, std::range_error );

	long result1{ eint_plus(lmin, 1L) };
	long result2{ eint_plus(lmax, -1L) };
	long result3{ eint_plus(lmin, 0L) };
	long result4{ eint_plus(lmax, 0L) };
	unsigned long result5{ eint_plus(ulmin, 1UL) };
	unsigned long result6{ eint_plus(ulmin, 0UL) };
	unsigned long result7{ eint_plus(ulmax, 0UL) };
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
	size_t c{ eint_minus(a,b) }; // good: no-compile-error
	// unsigned int xxx{ eint_minus(a,b) }; // good: compile-error
	EXPECT_THROW( { size_t d{ eint_minus(b,a) }; EXPECT_EQ(d,b-a); } , std::range_error );
	EXPECT_EQ(c,a-b);
	EXPECT_EQ(ULL(c) , ULL(a)-ULL(b));
}

using tup = std::tuple<
	char, unsigned char, signed char
	,short, unsigned short
	,int, unsigned int
	,long, unsigned long
	,long long, unsigned long long
>;

tup types{};

auto test_case = [](auto a, auto test_func){

	auto create_set = [](auto x){
		std::set<decltype(x)> x_set;
		x_set.insert(0);
		x_set.insert(1);
		x_set.insert(std::numeric_limits<decltype(x)>::max());
		x_set.insert(std::numeric_limits<decltype(x)>::max()-1);
		x_set.insert(std::numeric_limits<decltype(x)>::min()+1);
		x_set.insert(std::numeric_limits<decltype(x)>::max()/2);
		x_set.insert(std::numeric_limits<decltype(x)>::min()/2);
		x_set.insert(std::numeric_limits<decltype(x)>::min()+std::numeric_limits<decltype(x)>::max());
		return x_set;
	};

	std::set<decltype(a)> a_set = create_set(a);

	auto fun = [&](auto b){
		std::set<decltype(b)> b_set = create_set(b);
		for (auto x : a_set)
			for (auto y : b_set)
			{
				test_func(x, y);
			}
	};

	for_each(types, fun);
};

TEST(eint, eint_less_test) {
	EXPECT_TRUE(eint_less(1,2));
	EXPECT_TRUE(eint_less(0,1));
	EXPECT_TRUE(eint_less(-1,0));
	EXPECT_TRUE(eint_less(-2,-1));
	EXPECT_TRUE(eint_less(-1,2u));
	EXPECT_TRUE(eint_less(-1,2ul));
	EXPECT_TRUE(eint_less(-1l,2));
	EXPECT_TRUE(eint_less(-1ll,2));
	EXPECT_TRUE(eint_less(-1l,2ul));
	EXPECT_TRUE(eint_less(1l,2ul));
	EXPECT_TRUE(eint_less(1ll,2ul));
	EXPECT_FALSE(eint_less(1, 1));
	EXPECT_FALSE(eint_less(1, 1u));
	EXPECT_FALSE(eint_less(1, 1ull));
	EXPECT_FALSE(eint_less(1, 0ull));
	EXPECT_FALSE(eint_less(-1, -1));
	EXPECT_FALSE(eint_less(-1ll, -1));

	auto test_func = [](auto x, auto y){
		bool expected_result = static_cast<t_correct_int>(x) < static_cast<t_correct_int>(y);
		EXPECT_EQ(eint_less(x,y), expected_result);
	};

	auto test_case_eint_less = [&test_func](auto a){test_case(a, test_func);};

	for_each(types, test_case_eint_less);
}

TEST(eint, eint_equal_test) {
	EXPECT_FALSE(eint_equal(1,2));
	EXPECT_FALSE(eint_equal(0,1));
	EXPECT_FALSE(eint_equal(-1,0));
	EXPECT_FALSE(eint_equal(-2,-1));
	EXPECT_FALSE(eint_equal(-1,2u));
	EXPECT_FALSE(eint_equal(-1,2ul));
	EXPECT_FALSE(eint_equal(-1l,2));
	EXPECT_FALSE(eint_equal(-1ll,2));
	EXPECT_FALSE(eint_equal(-1l,2ul));
	EXPECT_FALSE(eint_equal(1l,2ul));
	EXPECT_FALSE(eint_equal(1ll,2ul));
	EXPECT_TRUE(eint_equal(1, 1));
	EXPECT_TRUE(eint_equal(1, 1u));
	EXPECT_TRUE(eint_equal(1, 1ull));
	EXPECT_TRUE(eint_equal(-1, -1ll));
	EXPECT_TRUE(eint_equal(-1, -1));
	EXPECT_TRUE(eint_equal(-1ll, -1));

	auto test_func = [](auto x, auto y){
		bool expected_result = static_cast<t_correct_int>(x) == static_cast<t_correct_int>(y);
		EXPECT_EQ(eint_equal(x,y), expected_result);
	};

	auto test_case_eint_equal = [&test_func](auto a){test_case(a, test_func);};

	for_each(types, test_case_eint_equal);
}

TEST(eint, eint_greater_test) {
	EXPECT_TRUE(eint_greater(std::numeric_limits<unsigned long>::max(), std::numeric_limits<long>::min()));
	EXPECT_TRUE(eint_greater(1, 0));
	EXPECT_TRUE(eint_greater(0, -1));
	EXPECT_TRUE(eint_greater(1u, 0));
	EXPECT_TRUE(eint_greater(0ul, -1));
	EXPECT_TRUE(eint_greater(1, 0u));
	EXPECT_TRUE(eint_greater(0, -1ll));
	EXPECT_TRUE(eint_greater(1, 0ull));
	EXPECT_TRUE(eint_greater(0ull, -1));
	EXPECT_FALSE(eint_greater(1,2));
	EXPECT_FALSE(eint_greater(0,1));
	EXPECT_FALSE(eint_greater(-1,0));
	EXPECT_FALSE(eint_greater(-2,-1));
	EXPECT_FALSE(eint_greater(-1,2u));
	EXPECT_FALSE(eint_greater(-1,2ul));
	EXPECT_FALSE(eint_greater(-1l,2));
	EXPECT_FALSE(eint_greater(-1ll,2));
	EXPECT_FALSE(eint_greater(-1l,2ul));
	EXPECT_FALSE(eint_greater(1l,2ul));
	EXPECT_FALSE(eint_greater(1ll,2ul));
	EXPECT_FALSE(eint_greater(1, 1));
	EXPECT_FALSE(eint_greater(1, 1u));
	EXPECT_FALSE(eint_greater(1, 1ull));
	EXPECT_FALSE(eint_greater(-1, -1));
	EXPECT_FALSE(eint_greater(-1ll, -1));

	auto test_func = [](auto x, auto y){
		bool expected_result = static_cast<t_correct_int>(x) > static_cast<t_correct_int>(y);
		EXPECT_EQ(eint_greater(x,y), expected_result);
	};

	auto test_case_eint_greater = [&test_func](auto a){test_case(a, test_func);};

	for_each(types, test_case_eint_greater);
}
