#include "gtest/gtest.h"
#include <exception>
#include <tnetdbg.hpp>

TEST(_templ, _unnamed_) {

	EXPECT_THROW( { std::vector<int> v(5); auto x = v.at(5); pfp_dbg1(x); } , std::out_of_range );
	EXPECT_EQ(2+2,4);
	EXPECT_NE(2+2,5);

}

