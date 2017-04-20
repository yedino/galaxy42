#include "gtest/gtest.h"
#include <iostream>

#include <libs0.hpp> // for debug etc

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
	for (int i=9; i<=300; ++i) EXPECT_THROW( { auto color = int_to_enum<t_color_components>(i);  _UNUSED(color); } , std::exception );

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
	auto gender_0 = int_to_enum<t_gender>(0);
	auto gender_1 = int_to_enum<t_gender>(1);

	EXPECT_THROW( { auto gender_2 = int_to_enum<t_gender>(2);  _UNUSED(gender_2); } , std::exception ); // there are no other genders

	_UNUSED(gender_0);
	_UNUSED(gender_1);
}

