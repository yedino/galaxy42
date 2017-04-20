#include "gtest/gtest.h"
#include <iostream>
#include <set>

#include <utils/misc.hpp> // <- we're testing this
#include "tnetdbg.hpp"

// ===========================================================================================================
// example of enum-class

enum class t_color_components { red=0, green=1, blue=2, ultra_violet, infra_red, joy, happiness, terror, dazzle  };
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

	EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(0);  _UNUSED(color); } );
	EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(2);  _UNUSED(color); } );
	for (int i=0; i<=8; ++i) EXPECT_NO_THROW( { auto color = int_to_enum<t_color_components>(i);  _UNUSED(color); } );
	for (int i=9; i<=300; ++i) EXPECT_THROW( { auto color = int_to_enum<t_color_components>(i);  _UNUSED(color); } , std::exception ); // no such base colors
}

// ===========================================================================================================
// non-class enum (deprecated)

enum t_gender { male=1, female=0 };
inline bool enum_is_valid_value(t_gender value) {
	switch (value) {
		case male:
		case female:
		return true;
	}
	return false;
}

TEST(stdplus_misc, function_enum_to_int) {
	EXPECT_NO_THROW( { auto gender = int_to_enum<t_gender>(0);	_UNUSED(gender); } );
	EXPECT_NO_THROW( { auto gender = int_to_enum<t_gender>(1);	_UNUSED(gender); } );
	EXPECT_THROW( { auto gender = int_to_enum<t_gender>(2);  _UNUSED(gender); } , std::exception ); // there are no other genders
}

// ===========================================================================================================

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

TEST(stdplus_misc, function_enum_to_int__enumclass_match_after_conversion) {
	{ auto en = int_to_enum<t_contact>(0);	EXPECT_EQ( en , t_contact::email ); }
	{ auto en = int_to_enum<t_contact>(1);	EXPECT_EQ( en , t_contact::phone ); }
	{ auto en = int_to_enum<t_contact>(5);	EXPECT_EQ( en , t_contact::sms ); }
	{ auto en = int_to_enum<t_contact>(6);	EXPECT_EQ( en , t_contact::jabber ); }
}

TEST(stdplus_misc, function_enum_to_int__enumclass_with_holes) {
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(0);  _UNUSED(en); } );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(1);  _UNUSED(en); } );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(2);  _UNUSED(en); } , std::exception );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(3);  _UNUSED(en); } , std::exception );
	EXPECT_THROW( { auto en = int_to_enum<t_contact>(4);  _UNUSED(en); } , std::exception );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(5);  _UNUSED(en); } );
	EXPECT_NO_THROW( { auto en = int_to_enum<t_contact>(6);  _UNUSED(en); } );
}

TEST(stdplus_misc, function_enum_to_int__enumclass_overflow) {
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




