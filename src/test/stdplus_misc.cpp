#include "gtest/gtest.h"
#include <iostream>
#include <set>

#include <utils/misc.hpp> // <- we're testing this
#include "tnetdbg.hpp"
#include <utility>
#include "my_tests_lib.hpp"

// ===========================================================================================================
// example of enum-class

enum class t_color_components { red=0, green=1, blue=2, ultra_violet=3, infra_red=4, joy=5, happiness=6, terror=-666, dazzle=7  };
/// ^ main color components, as seen by Mantis Shrimp https://en.wikipedia.org/wiki/Mantis_shrimp#Eyes
inline bool enum_is_valid_value(t_color_components value) {
	switch (value) {
		case t_color_components::red :
		case t_color_components::green :
		case t_color_components::blue :
		case t_color_components::ultra_violet :
		case t_color_components::infra_red :
		case t_color_components::joy :
		case t_color_components::happiness :
		case t_color_components::terror :
		case t_color_components::dazzle :
		return true;
	}
	return false;
}

TEST(stdplus_misc, function_enum_to_int__enumclass) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(0);  _UNUSED(color); } );
	EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(2);  _UNUSED(color); } );
	EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(-666);  _UNUSED(color); } );
	for (int i=0; i<=7; ++i) EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(i);  _UNUSED(color); } );
	for (int i=9; i<=300; ++i) EXPECT_THROW( { auto color = int_to_enum<t_color_components>(i);  _UNUSED(color); } , std::exception ); // no such base colors
}

// ===========================================================================================================
// non-class enum (deprecated)

// constant 35.5M is the production cost of https://en.wikipedia.org/wiki/Boeing_AH-64_Apache (variant E)
enum t_gender { female=1, male=0, ah64=35500000 };
inline bool enum_is_valid_value(t_gender value) {
	switch (value) {
		case female:
		case male:
		case ah64:
		return true;
	}
	return false;
}

TEST(stdplus_misc, function_enum_to_int) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	EXPECT_NO_THROW( { auto gender = int_to_enum<t_gender>(0);	_UNUSED(gender); } );
	EXPECT_NO_THROW( { auto gender = int_to_enum<t_gender>(1);	_UNUSED(gender); } );
	EXPECT_NO_THROW( { auto gender = int_to_enum<t_gender>(35500000);	_UNUSED(gender); } );
	EXPECT_THROW( { auto gender = int_to_enum<t_gender>(3);  _UNUSED(gender); } , std::exception ); // there is no gender number 3
}

// ===========================================================================================================
// tests for value overflow/wrapping - short underlying type of enum

enum class t_contact : unsigned char { email=0, phone=1, sms=5, jabber=6 };
inline bool enum_is_valid_value(t_contact value) {
	switch (value) {
		case t_contact::email:
		case t_contact::phone:
		case t_contact::sms:
		case t_contact::jabber:
		return true;
	}
	return false;
}

TEST(stdplus_misc, function_enum_to_int__exception_type_overflow_or_invalid) {
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(3);  _UNUSED(en); } , std::invalid_argument );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(300);  _UNUSED(en); } , std::overflow_error ); // value wraps
}

TEST(stdplus_misc, function_enum_to_int__enumclass_match_after_conversion) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	{ auto en = int_to_enum<t_contact>(0);	EXPECT_EQ( en , t_contact::email ); }
	{ auto en = int_to_enum<t_contact>(1);	EXPECT_EQ( en , t_contact::phone ); }
	{ auto en = int_to_enum<t_contact>(5);	EXPECT_EQ( en , t_contact::sms ); }
	{ auto en = int_to_enum<t_contact>(6);	EXPECT_EQ( en , t_contact::jabber ); }
}

TEST(stdplus_misc, function_enum_to_int__enumclass_with_holes) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(0);  _UNUSED(en); } );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(1);  _UNUSED(en); } );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(2);  _UNUSED(en); } , std::exception );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(3);  _UNUSED(en); } , std::exception );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(4);  _UNUSED(en); } , std::exception );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(5);  _UNUSED(en); } );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(6);  _UNUSED(en); } );
}

TEST(stdplus_misc, function_enum_to_int__enumclass_overflow) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	EXPECT_THROW( { auto en = int_to_enum<t_contact>( 0+256 );  _UNUSED(en); } , std::exception ); // wrap-around back to valid
	EXPECT_THROW( { auto en = int_to_enum<t_contact>( -1 );  _UNUSED(en); } , std::exception ); // wrap  of unsigned

	std::set<int> values;
	auto vmax = std::numeric_limits<int>::max();
	auto vmin = std::numeric_limits<int>::min();
	// values around max (and half), min (and half), and zero:
	for (int i=0; i<=+1000; ++i) { values.insert( vmin + i ); values.insert( vmin/2 +i ); }
	for (int i=-1000; i<=+1000; ++i) { values.insert( i ); }
	for (int i=-1000; i<=0; ++i) { values.insert( vmax + i ); values.insert( vmax/2 +i ); }

	for (auto i:values) {
		if ( (i==0) || (i==1) || (i==5) || (i==6) ) continue;
		EXPECT_THROW( { auto en = int_to_enum<t_contact>( i );  _UNUSED(en); } , std::exception );
	}
}

template<typename T, bool = std::is_integral<T>::value>
class c_test_enum
{
public:
	enum class t_enum_numeric_limits : T {
		min = std::numeric_limits<T>::min(),
		two = 2,
		max_half = std::numeric_limits<T>::max()/2,
		max = std::numeric_limits<T>::max()
	};
};

template<typename T>
inline bool enum_is_valid_value(T value) {
	using t_enum = T;
	switch (value) {
		case t_enum::min:
		case t_enum::two:
		case t_enum::max_half:
		case t_enum::max:
		return true;
	}
	return false;
}

template<typename T>
void test_case_enum_numeric_limits(c_test_enum<T>){
	using t_enum = typename c_test_enum<T>::t_enum_numeric_limits;
	EXPECT_EQ(t_enum::min, int_to_enum<t_enum>(std::numeric_limits<T>::min()));
	EXPECT_EQ(t_enum::max, int_to_enum<t_enum>(std::numeric_limits<T>::max()));
	EXPECT_EQ(t_enum::max_half, int_to_enum<t_enum>(std::numeric_limits<T>::max()/2));
	EXPECT_EQ(t_enum::two, int_to_enum<t_enum>(2));
	EXPECT_THROW(int_to_enum<t_enum>(std::numeric_limits<T>::max() - 1), std::exception );
	EXPECT_THROW(int_to_enum<t_enum>(std::numeric_limits<T>::min() + 1), std::exception );
	EXPECT_THROW(int_to_enum<t_enum>(3), std::exception ); // assume this test is not run on underlying type with max()==4

	EXPECT_THROW(int_to_enum<t_enum>( std::numeric_limits<long long unsigned>::max() - 5 ), std::exception );
}

TEST(stdplus_misc, function_enum_to_int_numeric_limits)
{
	using tup = std::tuple<
		char, unsigned char, signed char
		,short, unsigned short
		,int, unsigned int
		,long, unsigned long
		,long long, unsigned long long
	>;

	tup types{};
	for_each(types, [](auto type){c_test_enum<decltype(type)> test; test_case_enum_numeric_limits(test);});
}

// ===========================================================================================================
// example of enum-class with other underlying integer

enum class t_vehicle : long int { car=1900555, ship=111222333 };
/// ^ main color components, as seen by Mantis Shrimp https://en.wikipedia.org/wiki/Mantis_shrimp#Eyes
inline bool enum_is_valid_value(t_vehicle value) {
	switch (value) {
		case t_vehicle::car :
		case t_vehicle::ship :
		return true;
	}
	return false;
}

void fun_taking_short(short int s) {
	_info("short: " << s);
}

TEST(stdplus_misc, function_int_to_enum) {
	g_dbg_level_set(150, "reduce warnings spam from tests (int_to_enum etc)");

	t_vehicle myvalue{ t_vehicle::car };

	auto int1 = enum_to_int( myvalue );
	// short int int2{ enum_to_int( myvalue ) } ; // only warning
	// short int int3{ enum_to_int( myvalue ) } ;
	// int3 = enum_to_int( myvalue ) ; // warning only
	fun_taking_short( enum_to_int( t_vehicle::ship ) );
	// fun_taking_short( enum_to_int<short int>( t_vehicle::ship ) ); // warning
	_info(int1);
	// _info(int2);
	// _info(int3);

}


