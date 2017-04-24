#include "gtest/gtest.h"
#include <iostream>
#include <exception>

#include <libs0.hpp> // for debug etc

TEST(_templ, _unnamed_) {

	EXPECT_THROW( { vector<int> v(5); auto x = v.at(5); _dbg1(x); } , std::out_of_range );
	EXPECT_EQ(2+2,4);
	EXPECT_NE(2+2,5);

}

