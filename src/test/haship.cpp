// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include <boost/filesystem.hpp>
#include "../haship.hpp"


TEST(haship, creation_zero) {
	c_haship_addr x,y;

	for (int i=0; i<16; ++i) EXPECT_EQ(x.at(i), 0);
	for (int i=0; i<16; ++i) EXPECT_EQ(y.at(i), 0);
	EXPECT_EQ(x,y);
}

