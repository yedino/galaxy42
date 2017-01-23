// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../strings_utils.hpp"

TEST(string_utils, string_as_hex_create_from_string) {
	EXPECT_NO_THROW(string_as_hex());
	EXPECT_NO_THROW(string_as_hex("FD42"));
	EXPECT_NO_THROW(string_as_hex("fd42"));
	EXPECT_NO_THROW(string_as_hex("FD42gggggggg"));
	EXPECT_NO_THROW(string_as_hex("sldfjsfSFD>?!@#@!#$#$^%$&"));
	EXPECT_NO_THROW(string_as_hex(""));
	string_as_hex str("04DF409768A");
	EXPECT_NO_THROW(string_as_hex(str));
}

TEST(string_utils, string_as_hex_compare) {
	EXPECT_TRUE(string_as_hex() == string_as_hex());
	EXPECT_TRUE(string_as_hex("FD42") == string_as_hex("FD42"));
	EXPECT_FALSE(string_as_hex("FD42") == string_as_hex("fd42"));
	string_as_hex str("14DF4");
	EXPECT_TRUE(str == string_as_hex(str));
	string_as_hex str2("sldfjsfSFD>?!@#@!#$#$^%$&");
	EXPECT_TRUE(string_as_hex("sldfjsfSFD>?!@#@!#$#$^%$&") == str2);
	EXPECT_FALSE(str == str2);
}

TEST(string_utils, int_to_hex_char) {
	EXPECT_EQ(int2hexchar(0), '0');
	EXPECT_EQ(int2hexchar(1), '1');
	EXPECT_EQ(int2hexchar(2), '2');
	EXPECT_EQ(int2hexchar(3), '3');
	EXPECT_EQ(int2hexchar(4), '4');
	EXPECT_EQ(int2hexchar(5), '5');
	EXPECT_EQ(int2hexchar(6), '6');
	EXPECT_EQ(int2hexchar(7), '7');
	EXPECT_EQ(int2hexchar(8), '8');
	EXPECT_EQ(int2hexchar(9), '9');
	EXPECT_EQ(int2hexchar(10), 'a');
	EXPECT_EQ(int2hexchar(11), 'b');
	EXPECT_EQ(int2hexchar(12), 'c');
	EXPECT_EQ(int2hexchar(13), 'd');
	EXPECT_EQ(int2hexchar(14), 'e');
	EXPECT_EQ(int2hexchar(15), 'f');
	EXPECT_THROW(int2hexchar(16), std::invalid_argument);
	EXPECT_THROW(int2hexchar(20), std::invalid_argument);
	EXPECT_THROW(int2hexchar(255), std::invalid_argument);
	EXPECT_THROW(int2hexchar(100), std::invalid_argument);
}

TEST(string_utils, hex_char_to_int) {
	EXPECT_EQ(hexchar2int('0'), 0);
	EXPECT_EQ(hexchar2int('1'), 1);
	EXPECT_EQ(hexchar2int('2'), 2);
	EXPECT_EQ(hexchar2int('3'), 3);
	EXPECT_EQ(hexchar2int('4'), 4);
	EXPECT_EQ(hexchar2int('5'), 5);
	EXPECT_EQ(hexchar2int('6'), 6);
	EXPECT_EQ(hexchar2int('7'), 7);
	EXPECT_EQ(hexchar2int('8'), 8);
	EXPECT_EQ(hexchar2int('9'), 9);
	EXPECT_EQ(hexchar2int('a'), 10);
	EXPECT_EQ(hexchar2int('b'), 11);
	EXPECT_EQ(hexchar2int('c'), 12);
	EXPECT_EQ(hexchar2int('d'), 13);
	EXPECT_EQ(hexchar2int('e'), 14);
	EXPECT_EQ(hexchar2int('f'), 15);
	EXPECT_THROW(hexchar2int(0), std::invalid_argument);
	EXPECT_THROW(hexchar2int('h'), std::invalid_argument);
	EXPECT_THROW(hexchar2int('z'), std::invalid_argument);
	EXPECT_THROW(hexchar2int('*'), std::invalid_argument);
	EXPECT_THROW(hexchar2int('^'), std::invalid_argument);
	EXPECT_THROW(hexchar2int('%'), std::invalid_argument);
	EXPECT_THROW(hexchar2int('F'), std::invalid_argument);
	EXPECT_THROW(hexchar2int(127), std::invalid_argument);
}

TEST(string_utils, double_hex_char_to_int) {
	EXPECT_EQ(doublehexchar2int("00"), 0);
	EXPECT_EQ(doublehexchar2int("01"), 1);
	EXPECT_EQ(doublehexchar2int("02"), 2);
	EXPECT_EQ(doublehexchar2int("03"), 3);
	EXPECT_EQ(doublehexchar2int("10"), 16);
	EXPECT_EQ(doublehexchar2int("11"), 17);
	EXPECT_EQ(doublehexchar2int("fe"), 254);
	EXPECT_EQ(doublehexchar2int("ff"), 255);
	EXPECT_THROW(doublehexchar2int(""), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("fff"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("124"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("FD"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("1A"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("D3"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("ksjdhfksdhf"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("f%"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("*8"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("%$"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("zz"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("g7"), std::invalid_argument);
	EXPECT_THROW(doublehexchar2int("h0"), std::invalid_argument);
}

TEST(string_utils, string_as_bin_create) {
	EXPECT_NO_THROW(string_as_bin());
	EXPECT_NO_THROW(string_as_bin("01010101010"));
	EXPECT_NO_THROW(string_as_bin("kjsdfhksdhf"));
	const std::string str("fsd2132134214$%^&%$^&%^");
	EXPECT_NO_THROW(string_as_bin(str.data(), str.size()));
	string_as_hex hex_str("FD42gggggggg");
	EXPECT_NO_THROW(string_as_bin(hex_str));
}

TEST(string_utils, string_as_bin_operators) {
	string_as_bin str1("aaaaaa");
	string_as_bin str2("bbbbbb");
	EXPECT_FALSE(str1 == str2);
	EXPECT_TRUE(str1 != str2);

	string_as_bin str1_copy = str1;
	EXPECT_TRUE(str1 == str1_copy);
	EXPECT_FALSE(str1 != str1_copy);

	string_as_bin str12 = str1;
	EXPECT_NO_THROW(str12 += str2);
	EXPECT_EQ(str12, str1 + str2);
	EXPECT_NE(str12, str2 + str1);

	string_as_bin str3("cccccc");
	std::string str4("dddddd");
	string_as_bin str34 = str3;
	EXPECT_NO_THROW(str34 += str4);
	EXPECT_EQ(str34, str3 + str4);

	EXPECT_TRUE(str1 < str2);
	EXPECT_FALSE(str2 < str1);
	EXPECT_FALSE(str1 < str1);
}

TEST(string_utils, string_as_dbg_create) {
	EXPECT_NO_THROW(string_as_dbg());

	string_as_bin bin("w234rgt554tgeargt45");
	EXPECT_NO_THROW(string_as_dbg(bin, e_debug_style_object));
	EXPECT_NO_THROW(string_as_dbg(bin, e_debug_style_short_devel));
	EXPECT_NO_THROW(string_as_dbg(bin, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(string_as_dbg(bin, e_debug_style_big));

	std::string str("10101fd45");
	EXPECT_NO_THROW(string_as_dbg(str.data(), str.size(), e_debug_style_object));
	EXPECT_NO_THROW(string_as_dbg(str.data(), str.size(), e_debug_style_short_devel));
	EXPECT_NO_THROW(string_as_dbg(str.data(), str.size(), e_debug_style_crypto_devel));
	EXPECT_NO_THROW(string_as_dbg(str.data(), str.size(), e_debug_style_big));

	std::array<char, 4> arr {{'a', 'b', 'c', 'd'}};
	EXPECT_NO_THROW(string_as_dbg(arr.begin(), arr.end(), e_debug_style_object));
	EXPECT_NO_THROW(string_as_dbg(arr.begin(), arr.end(), e_debug_style_short_devel));
	EXPECT_NO_THROW(string_as_dbg(arr.begin(), arr.end(), e_debug_style_crypto_devel));
	EXPECT_NO_THROW(string_as_dbg(arr.begin(), arr.end(), e_debug_style_big));

	EXPECT_NO_THROW(string_as_dbg(arr));
}

TEST(string_utils, to_debug) {
	std::string str("fbhiw7wo308w938ru03urfw!@#!@#a");
	EXPECT_NO_THROW(to_debug(str, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(str));
	str.clear();
	EXPECT_NO_THROW(to_debug(str, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(str, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(str));

	char c = 'a';
	EXPECT_NO_THROW(to_debug(c, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(c));
	c = 0;
	EXPECT_NO_THROW(to_debug(c, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(c, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(c));

	string_as_bin bin("kdhsaiwe7hdf97e");
	EXPECT_NO_THROW(to_debug(bin, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(bin, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(bin, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(bin, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(bin));

	string_as_bin bin_empty;
	EXPECT_NO_THROW(to_debug(bin_empty, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(bin_empty, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(bin_empty, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(bin_empty, e_debug_style_big));
	EXPECT_NO_THROW(to_debug_b(bin_empty));
}

TEST(string_utils, debug_simple_hash) {
	EXPECT_NO_THROW(debug_simple_hash(""));
	EXPECT_NO_THROW(debug_simple_hash("sadfsdfa"));
	EXPECT_NO_THROW(debug_simple_hash("123123445"));
	EXPECT_NO_THROW(debug_simple_hash("!@#3e234!RDQWER"));
	EXPECT_NO_THROW(debug_simple_hash(";;l[['l[l;"));
}
