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

void addr_is_same(string s1, string s2, bool first_element_was_self_checked=false) {
	auto a1 = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot(), s1);
	auto a2 = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot(), s2);

	EXPECT_EQ(a1,a2);

	if (!first_element_was_self_checked) EXPECT_EQ(a1,a1);
	EXPECT_EQ(a2,a2);
}

void addr_is_same(string s1, vector<string> s2tab) {
	addr_is_same(s1,s1); // self-check on first element
	for(const auto & s2 : s2tab) addr_is_same(s1,s2,true);
}

void addr_is_invalid(string s1) {
	auto fun1 = [=]() {
		auto a1 = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot(), s1);
		_UNUSED(a1);
	};
	EXPECT_THROW( fun1() , std::invalid_argument);
}

TEST(haship, speciall_addresses) {
	addr_is_same("0000:0000:0000:0000:0000:0000:0000:0000",
		{
			"::",
			"::0",
			"0::",
			"::0",
			"::0:0",
			"0:0::",
			"0:000::",
		}
	);

	addr_is_same("0000:0000:0000:0000:0000:0000:0000:0001",
		{
			"::1",
			"::0:1",
			"::0:1",
			"0::0:1",
			"0:0::0:1",
			"0:000::0:1",
		}
	);

	addr_is_same("ffff:0000:0000:0000:0000:0000:0000:0000",
		{
			"ffff::",
			"ffff:0::",
			"ffff:000::",
			"ffff:000::",
			"ffff:000:0::",
			"ffff:000::0",
			"ffff::000:0",
		}
	);
}

TEST(haship, basic_conversions) {
	addr_is_same("00ff:0000:0000:0000:0000:0000:0001:0004",
		{
			"00ff:0000:0000:0000:0000:0000:0001:0004",

			"ff::0000:0000:0001:4",
			"ff:0000:0000::0001:4",

			"ff::0:0000:0001:4",
			"ff::0000:0:0001:4",

			"ff::0:0001:4",
			"ff::0000:0001:4",

			"ff::0001:4",
			"ff::0001:0004",
			"ff::01:0004",
			"ff::01:004",
			"ff::01:04",
			"ff::1:04",
			"ff::01:4",
			"ff::1:4",
		}
	);
}

TEST(haship, invalid_addresses) {
	addr_is_invalid("ffff:0000:0000:0000:0000:0000:0000:0000:f");
	addr_is_invalid("ffff:0000:0000:0000:0000:0000:0000:0000:");
	addr_is_invalid("ffff:0000:0000:0000:0000:0000:0000:0000::");
	addr_is_invalid("");
	addr_is_invalid(":");
	addr_is_invalid(":::");
	addr_is_invalid("::::");
	addr_is_invalid("::0::");
	addr_is_invalid("ffff::aaaa::bbbb");
}

