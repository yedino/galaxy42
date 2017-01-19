
#include "my_tests_lib.hpp"

#include "../xint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>

#if USE_BOOST_MULTIPRECISION

typedef long double t_correct1;




TEST(xint,normal_use_init) {
	xint a; a=0;
	a=1;
	a=100;
	UNUSED(a);
}

TEST(xint,normal_use_ostring) {
	xint a; a=123;
	std::ostringstream oss; oss<<a;
	EXPECT_EQ( oss.str() , "123" );
}


TEST(xint,normal_use_assign) {
	xint a; a=0;
	a=1; EXPECT_EQ(a,1);
	a=100; EXPECT_EQ(a,100);
	a=-1; EXPECT_EQ(a,-1);
	a=-100; EXPECT_EQ(a,-100);
}

TEST(xint,normal_use_assign2) {
	xint a=42, b=100, c(a+b);
	EXPECT_EQ(c, 142);
	EXPECT_EQ(c, xint(142));
	EXPECT_EQ(c, c);
}

TEST(xint,normal_use_compare) {
	xint a=42, b=100, c(a+b);
	EXPECT_EQ(c, 142);
	EXPECT_NE(c, a);
	EXPECT_NE(c, b);
	EXPECT_EQ(c, a+b);
	EXPECT_EQ(a, a);
	EXPECT_EQ(c, c);

	EXPECT_TRUE( (a==a) );
	EXPECT_TRUE( (b==b) );
	EXPECT_TRUE( (c==c) );

	EXPECT_TRUE( (a<b) );
	EXPECT_TRUE(!(b<a) );
	EXPECT_TRUE( (a<=b) );
	EXPECT_TRUE(!(b<=a) );
	EXPECT_TRUE( (a<=a) );

	EXPECT_TRUE( (b>a) );
	EXPECT_TRUE(!(a>b) );
	EXPECT_TRUE( (b>=a) );
	EXPECT_TRUE(!(a>=b) );
	EXPECT_TRUE( (b>=b) );
}


TEST(xint,normal_use_inc) {
	xint a;
	a=0; EXPECT_EQ(a,0);
	a++; EXPECT_EQ(a,1);
	a++; EXPECT_EQ(a,2);
	EXPECT_EQ(a++,2); EXPECT_EQ(a,3);
}

TEST(xint,normal_use_dec) {
	xint a;
	a=0; EXPECT_EQ(a,0);
	a--; EXPECT_EQ(a,-1);
	a--; EXPECT_EQ(a,-2);
	EXPECT_EQ(a--,-2); EXPECT_EQ(a,-3);
}

TEST(xint,normal_use_op4assign) {
	xint a=100, b=30, c=8;
	{ xint d(a+b); EXPECT_EQ(d, 130); }
	{ xint d(a+b); EXPECT_EQ(d, xint(130)); }
	{ xint d(a+b); EXPECT_EQ(d, (a+b)); }
	{ xint d(a+b+c); EXPECT_EQ(d, 138); }

	{ xint d(a-b); EXPECT_EQ(d, 70); }
	{ xint d(a-b); EXPECT_EQ(d, xint(70)); }
	{ xint d(a-b); EXPECT_EQ(d, (a-b)); }

	{ xint d(a*b); EXPECT_EQ(d, 3000); }
	{ xint d(a*b); EXPECT_EQ(d, xint(3000)); }
	{ xint d(a*b); EXPECT_EQ(d, (a*b)); }

	{ xint d(a/b); EXPECT_EQ(d, 3); }
	{ xint d(a/b); EXPECT_EQ(d, xint(3)); }
	{ xint d(a/b); EXPECT_EQ(d, (a/b)); }
}

TEST(xint,normal_use_belowzero) {
	xint a=100, b=30;
	{ xint d(b-a); EXPECT_EQ(d, -70); }
	{ xint d(-a + b); EXPECT_EQ(d, -70); }
	{ xint d(-a + -b); EXPECT_EQ(d, -130); }
	{ xint d(-a + -5); EXPECT_EQ(d, -105); }
}


TEST(xint,can_assign) {
	xint a;
	/*
	basic_xint b;
	t_correct_int corr1(9LL);
	_warn( std::numeric_limits<decltype(a)>::max() );
	_warn( std::numeric_limits<decltype(b)>::max() );
	*/

	EXPECT_TRUE( overflow_impossible_in_assign(a, 0LL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, 9LL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, 100LL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, 10000LL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, 0xFFFFLL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, 0xFFFFFFFFLL) );
	EXPECT_TRUE( overflow_impossible_in_assign(a, t_correct_int(0xFFFFFFFFLL)) ); // TODO
	EXPECT_TRUE ( overflow_impossible_in_assign(a, t_correct_int(0xFFFFFFFFFFFFFFFFLL)-1) ); // TODO
	EXPECT_FALSE( overflow_impossible_in_assign(a, t_correct_int(0xFFFFFFFFFFFFFFFFLL)+1) ); // TODO
	EXPECT_FALSE( overflow_impossible_in_assign(a, t_correct_int(0xFFFFFFFFFFFFFFFFLL)+2) ); // TODO
	EXPECT_FALSE( overflow_impossible_in_assign(a, t_correct_int(0xFFFFFFFFFFFFFFFFLL)+200) ); // TODO
	UNUSED(a);
}

TEST(xint,normal_use_op4assign_loop) {
	return ; // XXX

	vector<uint64_t> tab_int({0,1,2,3,42, 256, 1024,
		0xFFFF, 0xDEADBEEF, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFFULL});
	//for (int i=0; i<100; ++i) tab_int.push_back(i);

	g_dbg_level_set(50,"Details of test");

	vector<t_correct_int> tab;
	for (auto v:tab_int) {
		tab.push_back(v);
		tab.push_back(-t_correct_int(v));
	}
	decltype(tab) tab2;
	tab2=tab;
	for (auto v:tab) { tab2.push_back(v-1); tab2.push_back(v+1);	}
	tab=tab2;

	tab2=tab;
	for (auto v:tab) {
		tab2.push_back(v*2);
		tab2.push_back(v*3);
		tab2.push_back(v*256);
		tab2.push_back(v*10000000);
	}
	tab=tab2;

	tab2=tab;
	for (auto v:tab) { tab2.push_back(v-1); tab2.push_back(v+1);	}
	tab=tab2;

	{
		std::sort(tab.begin(), tab.end());
		auto last = std::unique(tab.begin(), tab.end());
		tab.erase(last, tab.end());
	}

	_note("Tab of numbers size: "<<tab.size());

	long count_fitting=0, count_xflov=0; // count operations that fit, and operations that would over/under-flow

	for (size_t i=0; i<tab.size(); ++i) {
		for (size_t j=0; j<tab.size(); ++j) {
			t_correct_int a_ok=tab.at(i);
			t_correct_int b_ok=tab.at(j);
			xint ab_type;

			bool fit_a = overflow_impossible_in_assign(ab_type,a_ok);
			bool fit_b = overflow_impossible_in_assign(ab_type,b_ok);

			if (!(fit_a && fit_b)) {
				EXPECT_THROW( { xint a(a_ok); xint b(b_ok); } , std::runtime_error );
			}
			else
			{
				xint a(a_ok); xint b(b_ok);
				xint c;

				#define THE_TEST( OPERATOR , OPERATOR_NAME ) do { \
				bool div_by_zero = (string("div")==OPERATOR_NAME) && (b_ok == 0); \
				bool operation_is_valid = !div_by_zero; \
				t_correct_int c_ok; \
				if (operation_is_valid) { c_ok = a_ok OPERATOR b_ok; } else c_ok=-1; \
				if (overflow_impossible_in_assign(c,c_ok) && operation_is_valid) { \
					_dbg2("Should be ok a="<<a<<" b="<<b); \
					++count_fitting; \
					c = a OPERATOR b; \
					EXPECT_EQ(c, c_ok); \
				} \
				else { \
					++count_xflov; \
					_dbg2("Should fail a="<<a<<" b="<<b); \
					EXPECT_THROW( { c = a OPERATOR b; } , std::runtime_error ); \
				} \
				} while(0)

				THE_TEST( + , "add" );
				THE_TEST( - , "sub" );
				THE_TEST( * , "mul" );
				THE_TEST( / , "div" );
				#undef THE_TEST


				#define THE_TEST( OPERATOR , OPERATOR_NAME ) do { \
				bool div_by_zero = (string("div")==OPERATOR_NAME) && (b_ok == 0); \
				bool operation_is_valid = !div_by_zero; \
				t_correct_int c_ok; \
				if (operation_is_valid) { c_ok = a_ok;  c_ok OPERATOR b_ok; } else c_ok=-1; \
				if (overflow_impossible_in_assign(c,c_ok) && operation_is_valid) { \
					_dbg2("Should be ok a="<<a<<" b="<<b); \
					++count_fitting; \
					c = a;  c OPERATOR b; \
					EXPECT_EQ(c, c_ok); \
				} \
				else { \
					++count_xflov; \
					_dbg2("Should fail a="<<a<<" b="<<b); \
					EXPECT_THROW( { c = a;  c OPERATOR b; } , std::runtime_error ); \
				} \
				} while(0)

				THE_TEST( += , "add" );
				THE_TEST( -= , "sub" );
				THE_TEST( *= , "mul" );
				THE_TEST( /= , "div" );
				#undef THE_TEST

			} // fitting initial values
		}
	}
	_note("Over/under-flow count: " << count_xflov);
	EXPECT_GT(count_xflov, 248);
	_note("Normal operation count: " << count_fitting);
	EXPECT_GT(count_fitting, 16393);

}


namespace test_xint {
namespace detail {



/*
template<typename T_INT,
typename std::enable_if<std::is_signed<T_INT>{}>::type* = nullptr >
signed long long int round_co(t_correct1 v) {
	return static_cast<signed long long int>( std::round( v ) );
}

template<typename T_INT,
typename std::enable_if<! std::is_signed<T_INT>{}>::type* = nullptr >
unsigned long long int round_co(t_correct1 v) {
	return static_cast<unsigned long long int>( std::round( v ) );
}
*/
template<typename T_INT>
void math_tests_noproblem() {
	vector<int> testsize_tab = { 1, 2, 50, 1000, 10000 , 1000000 };
	for (auto testsize : testsize_tab) {
		T_INT a=0;
		int n=testsize;
		for (int i=1; i<n; ++i) a = a + i;
		t_correct_int n_corr = n;
		t_correct_int a_corr = (((n_corr-1)*1)* n_corr) /2;
		EXPECT_EQ(a, a_corr);
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
void math_tests_overflow_incr(T_INT val) { bool safetype = is_safe_int<T_INT>();
	// @hint: if this fails, then it can be called from code "generate_tests_for_types" go fix that
	T_INT a = val;	t_correct_int a_corr = a;
	auto func = [&]() { a_corr+=1; a++; } ;
	#define db do { _mark("safe="<<safetype<<"; a="<<a<<" a_corr="<<a_corr); } while(0)

	_note("Will increment first time: " << a << " (a_corr="<<a_corr<<")" );
	EXPECT_NO_THROW( func() );
	EXPECT_EQ(a,a_corr); // this should fit for given starting val
	// next icrement is problematic:
	_note("Will increment again: " << a );
	if (safetype) { EXPECT_THROW( func() , std::runtime_error ); }
	else { // should not fit for given val
		EXPECT_NO_THROW( func() ); // usafe type fils to throw
		EXPECT_NE(a , a_corr); // unsafe type has mathematically-invalid value
	}
	#undef db
}

template<typename T_INT>
void math_tests_overflow_decr(T_INT val) { bool safetype = is_safe_int<T_INT>();
	T_INT a = val;	t_correct_int a_corr = a;
	EXPECT_EQ(a,a_corr);
	auto func = [&]() { a_corr-=1; a--; } ;
	#define db do { _mark("safe="<<safetype<<"; a="<<a<<" a_corr="<<a_corr); } while(0)
	EXPECT_NO_THROW( func() );
	EXPECT_EQ(a,a_corr); // this should fit for given starting val
	// next icrement is problematic:
	if (safetype) {  EXPECT_THROW( func() , std::runtime_error ); }
	else { // should not fit for given val
		EXPECT_NO_THROW( func() ); // usafe type fils to throw
		EXPECT_NE(a , a_corr); // unsafe type has mathematically-invalid value
	}
	#undef db
}

} // namespace
} // namespace


// TODO move to general gtest_examples_test.cpp later (and rename from "xint")
TEST(xint, gtest_throw) {
	EXPECT_THROW( { throw std::runtime_error("testerror"); } , std::runtime_error );
}
TEST(xint, gtest_nothrow) {
	EXPECT_NO_THROW( { int a=5; int b=10; int c=55; if (c<a+b) throw std::runtime_error("testerror"); } );
}

TEST(xint, gtest_throw_lambda) {
	// EXPECT_NO_THROW( { int a=5, int b=10, int c=55; if (c>a+b) throw std::runtime_error("testerror"); } );
	auto func = []() { int a=5, b=10, c=55; if (c>a+b) throw std::runtime_error("testerror"); } ;
	EXPECT_THROW( func() , std::runtime_error );
}
TEST(xint, gtest_nothrow_lambda) {
	// EXPECT_NO_THROW( { int a=5, b=10, c=55; if (c>a+b) throw std::runtime_error("testerror"); } );
	auto func = []() { int a=5, b=10, c=55; if (c<a+b) throw std::runtime_error("testerror"); } ;
	EXPECT_NO_THROW( func() );
}

TEST(xint, math1) {
	test_xint::detail::math_tests_noproblem<long int>();
	test_xint::detail::math_tests_noproblem<unsigned long int>();
	test_xint::detail::math_tests_noproblem<xint>();
	test_xint::detail::math_tests_noproblem<uxint>();
}

//const test_xint::detail::t_correct_int maxni = 0xFFFFFFFFFFFFFFFF; // max "normal integer" on this platform
#define maxni 0xFFFFFFFFFFFFFFFFULL // max "normal integer" on this platform


//static_assert(maxni == std::numeric_limits<uint64_t>::max() , "Unexpected max size of normal integer");

#define generate_tests_for_types(FUNCTION, V1,V2,V3,V4) \
TEST(xint, FUNCTION ## _u_i) {	test_xint::detail::math_tests_ ## FUNCTION <uint64_t>(V1); } \
TEST(xint, FUNCTION ## _u_xint) {	test_xint::detail::math_tests_ ## FUNCTION <uxint>(V2); } \
TEST(xint, FUNCTION ## _s_i) {	test_xint::detail::math_tests_ ## FUNCTION <int64_t>(V3); } \
TEST(xint, FUNCTION ## _s_xint) {	test_xint::detail::math_tests_ ## FUNCTION <xint>(V4); }
// we use max_u64-1 for SIGNED xint too, because it can in fact express it it seems?

generate_tests_for_types( overflow_incr , maxni-1, maxni-1, maxni/2-1, xint(maxni-1) )
generate_tests_for_types( overflow_decr , +1, +1, -(maxni/2), -xint(maxni-1) )

TEST(xint, some_use) {
	xint a("0xFFFFFFFFFFFFFFFF");
	a--;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFE"));
	a--;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFD"));
	a++;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFE"));
	a++;
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );
	EXPECT_EQ(a , xint("0xFFFFFFFFFFFFFFFF"));

	a = xint("0x8888888888888888");
	a/=2;
	EXPECT_EQ(a , xint("0x4444444444444444"));
	a/=2;
	EXPECT_EQ(a , xint("0x2222222222222222"));
}

TEST(xint, range_u_incr) {
	typedef uxint T;
	T a("0xFFFFFFFFFFFFFFFE");
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
}
TEST(xint, range_u_decr) {
	typedef uxint T;
	T a("0x0000000000000001");
	EXPECT_NO_THROW( { a--; } );  EXPECT_EQ(a , T("0x0000000000000000"));
	EXPECT_THROW( { a--; } , std::runtime_error );	EXPECT_EQ(a , T("0x0000000000000000"));
	EXPECT_THROW( { a--; } , std::runtime_error );	EXPECT_EQ(a , T("0x0000000000000000"));
}

TEST(xint, range_s_incr) {
	typedef xint T;
	T a("0xFFFFFFFFFFFFFFFE");
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
	EXPECT_THROW( { a++; } , std::runtime_error );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
}
TEST(xint, range_s_decr) {
	typedef xint T;
	T b("0xFFFFFFFFFFFFFFFE");
	T a(0); a -= b;
	EXPECT_NO_THROW( { a--; } );
	EXPECT_THROW( { a--; } , std::runtime_error );
	EXPECT_THROW( { a--; } , std::runtime_error );
	EXPECT_NO_THROW( { a += b; } );
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , T("0x0000000000000000"));
	EXPECT_NO_THROW( { a++; } );	EXPECT_EQ(a , T("0x0000000000000001"));
	EXPECT_NO_THROW( { a+=b; } );	EXPECT_EQ(a , T("0xFFFFFFFFFFFFFFFF"));
}

TEST(xint, range_u_to_sizet) {
	size_t s1 = 0xFFFFFFFFFFFFFFFF - 10, s2=8, s3=2, s4=1;
	vector<int> tab10(10);
	vector<int> tabBig(10*1000000);
	ASSERT_EQ(tab10.size(),10u);
	size_t sm = 0xFFFFFFFFFFFFFFFF;
	uxint a = s1;
	xint as = s1;
	UNUSED(a);
	UNUSED(as);
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(s2)+uxint(s3)+uxint(s4); UNUSED(x); } , std::runtime_error );
	{               uxint x = uxint(s1)+uxint(s2)+uxint(s3);  EXPECT_EQ(x,sm); }
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(tab10.size())+uxint(s4); UNUSED(x); } , std::runtime_error );
	EXPECT_THROW( { uxint x = uxint(s1)+uxint(tab10.size())+1-1+1; UNUSED(x); } , std::runtime_error );
	{               uxint x = uxint(s1)+uxint(tab10.size());  EXPECT_EQ(x,sm); }

	EXPECT_THROW( { uxint x = uxint(tabBig.size()) * uxint(tabBig.size()) * uxint(tabBig.size()); UNUSED(x); } , std::runtime_error );
	//                              tabBig.size()  *       tabBig.size()
	EXPECT_THROW( { uxint x = xsize(tabBig) * xsize(tabBig) * xsize(tabBig); UNUSED(x); } , std::runtime_error );

	long int time_dell = 100000000000;
	EXPECT_THROW( { uxint x = xsize(tabBig) * uxint(1000000) * uxint(time_dell); UNUSED(x); } , std::runtime_error );
}

void someint(long long int x) {
	_mark("someint got: " << x);
}

template <typename T>
uxint make_uxint(T value) {	return numeric_cast<uint64_t>(value); }
template <typename T>
uxint make_xint(T value) {	return numeric_cast<int64_t>(value); }


TEST(xint, range_b_to_sizet) {
	size_t s1 = 0xFFFFFFFFFFFFFFFF - 10;//, s2=8, s3=2, s4=1;
	vector<int> tab10(10);
	vector<int> tabBig(10*1000000);
	ASSERT_EQ(tab10.size(),10u);
	// size_t sm = 0xFFFFFFFFFFFFFFFF;
	uxbigint points = s1, value=5000;
	points *= value;
	EXPECT_THROW( { uxint points_size( points ); size_t s( points_size ); UNUSED(s); }  , std::runtime_error );
	              { uxint points_size( points/value ); size_t s( points_size ); EXPECT_EQ(s,s1); }

	EXPECT_THROW( { size_t s( points ); UNUSED(s); }  , std::runtime_error );
	              { size_t s( points/value ); UNUSED(s); }

	EXPECT_THROW( { size_t s( xsize(tabBig)*xsize(tabBig)*xsize(tabBig) ); UNUSED(s); }  , std::runtime_error );
	              { size_t s( xsize(tabBig)*xsize(tabBig) ); UNUSED(s); }
}

TEST(xint, safe_create_float1) {
	auto func = [](bool negative) { float a=10000, b=10000000000, c=100000000;
		if (negative) a = -a;
		xint bonus(a*b*c); UNUSED(bonus);	} ;
	EXPECT_THROW( func(true)  , std::runtime_error );
	EXPECT_THROW( func(false)  , boost::numeric::bad_numeric_cast );
}
TEST(xint, safe_create_float2) { // obviously the same, other syntax as example
	auto func = [](bool negative) { float a=10000, b=10000000000, c=100000000;
		if (negative) a = -a;
		xint bonus = a*b*c; UNUSED(bonus);	} ;
	EXPECT_THROW( func(true)  , std::runtime_error );
	EXPECT_THROW( func(false)  , boost::numeric::bad_numeric_cast );
}
/*TEST(xint, safe_create_float_assign) {
	auto func = []() { float a=10000, b=10000000000, c=100000000;
		xint bonus;  bonus = a*b*c; UNUSED(bonus);	} ;
	EXPECT_THROW( func()  , boost::numeric::bad_numeric_cast );
}*/

// terminate called after throwing an instance of 'boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<std::overflow_error> >'
TEST(xint, safe_create_xint) {
	auto func = [](bool negative) { xint a=10000, b=10000000000, c=100000000;
		if (negative) a = -a;
		xint bonus(a*b*c*c*c); UNUSED(bonus);	} ;
		EXPECT_THROW( func(true)  , std::runtime_error );
		EXPECT_THROW( func(false)  , std::runtime_error );
}
TEST(xint, safe_create_xint2) { // obviously the same, other syntax as example
	auto func = [](bool negative) { xint a=10000, b=10000000000, c=100000000;
		if (negative) a = -a;
		xint bonus = a*b*c*c*c; UNUSED(bonus);	} ;
	EXPECT_THROW( func(true)  , std::runtime_error );
	EXPECT_THROW( func(false)  , std::runtime_error );
}
TEST(xint, safe_create_xint_assign) {
	auto func = [](bool negative) { xint a=10000, b=10000000000, c=100000000;
		if (negative) a = -a;
		xint bonus;  bonus = a*b*c*c*c; UNUSED(bonus);	} ;
	EXPECT_THROW( func(true)  , std::runtime_error );
	EXPECT_THROW( func(false)  , std::runtime_error );
}



#undef maxni



#else

#warning "USE_BOOST_MULTIPRECISION is disabled - so we can not test this part of code"

TEST(xint, disabled) {
	TEST_IS_DISABLED("Compilation options disabled USE_BOOST_MULTIPRECISION");
}

#endif


