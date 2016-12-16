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

TEST(haship, print){
  c_haship_addr empty_ip = c_haship_addr();
  
  ostringstream empty_haship;
  empty_ip.print(empty_haship);
  EXPECT_EQ(empty_haship.str(),"hip=0000:0000:0000:0000:0000:0000:0000:0000");
  
}

/*
TEST(haship, tag_pubkey){
  const c_haship_pubkey mykey='2001:0db8:0a0b:12f0:0000:0000:0000:0001';
  c_haship_addr key = c_haship_addr(c_haship_addr::tag_constr_by_hash_of_pubkey() , mykey);
  ostringstream haship;
  key.print(haship);
  std::cout<<"TEST\n\n\n"<<haship<<endl;
  EXPECT_EQ(haship.str(),"");
}
*/
TEST(haship, tag_dot){
  const t_ipv6dot mydot="2001:0db8:0a0b:12f0:0000:0000:0000:0001";
  c_haship_addr dot = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot() , mydot);
  ostringstream haship;
  dot.print(haship);
  EXPECT_EQ(haship.str(),"hip=2001:0db8:0a0b:12f0:0000:0000:0000:0001");
}
/*
TEST(haship, tag_bin){
  const t_ipv6bin mybin;
  c_haship_addr bin = c_haship_addr(c_haship_addr::tag_constr_by_addr_bin() , mybin);
  ostringstream haship;
  bin.print(haship);
  std::cout<<"TEST\n\n\n"<<haship.str()<<endl;
  //EXPECT_EQ(haship.str(),"");
}

TEST(haship, c_haship_pubkey){
  c_haship_pubkey empty_pubkey = c_haship_pubkey();
  ostringstream empty_pubkey_ostr;
  empty_pubkey.print(empty_pubkey_ostr);
  EXPECT_EQ(empty_pubkey_ostr.str(),"");
}

TEST(haship, string_as_bin){
  const string_as_bin pubkey_bin;
  c_haship_pubkey pubkey = c_haship_pubkey();
  ostringstream pubkey_bin_ostr;
  pubkey.print(pubkey_bin_ostr);
  EXPECT_EQ(pubkey_bin_ostr.str(),"");
}*/


TEST (haship, is_galaxy){
  c_haship_addr address1 = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot() , "2001:0db8:0a0b:12f0:0000:0000:0000:0001");
  EXPECT_FALSE(addr_is_galaxy(address1));
  c_haship_addr address2 = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot() , "fd42:0db8:0a0b:12f0:0000:0000:0000:0001");
  EXPECT_TRUE(addr_is_galaxy(address2));
}
