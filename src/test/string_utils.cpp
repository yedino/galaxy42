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

TEST(string_utils, to_debug_string) {
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
}

TEST(string_utils, to_debug_char) {
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
}

TEST(string_utils, to_debug_string_as_bin) {
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

TEST(string_utils, to_debug_unique_ptr) {
	std::unique_ptr<std::string> u_ptr;
	EXPECT_NO_THROW(to_debug(u_ptr));
	auto u_ptr2 = std::make_unique<std::string>("ksafh24098");
	auto u_ptr3 = std::make_unique<std::string>("ksafhhasgdijasgd");
	EXPECT_NO_THROW(to_debug(u_ptr2));
	EXPECT_NO_THROW(to_debug(u_ptr3));
	EXPECT_NE(to_debug(u_ptr), to_debug(u_ptr2));
	EXPECT_NE(to_debug(u_ptr2), to_debug(u_ptr3));
	EXPECT_NE(to_debug(u_ptr), to_debug(u_ptr3));
}

TEST(string_utils, to_debug_vector) {
	std::vector<int> vec1 = {1, 2, 5, 8, 4, 3, 4};
	std::vector<int> vec2;
	EXPECT_NO_THROW(to_debug(vec1, e_debug_style_object));
	EXPECT_NO_THROW(to_debug(vec2, e_debug_style_object));
	EXPECT_NE(to_debug(vec1, e_debug_style_object), to_debug(vec2, e_debug_style_object));

	EXPECT_NO_THROW(to_debug(vec1, e_debug_style_short_devel));
	EXPECT_NO_THROW(to_debug(vec2, e_debug_style_short_devel));
	EXPECT_NE(to_debug(vec1, e_debug_style_short_devel), to_debug(vec2, e_debug_style_short_devel));

	EXPECT_NO_THROW(to_debug(vec1, e_debug_style_crypto_devel));
	EXPECT_NO_THROW(to_debug(vec2, e_debug_style_crypto_devel));
	EXPECT_NE(to_debug(vec1, e_debug_style_crypto_devel), to_debug(vec2, e_debug_style_crypto_devel));

	EXPECT_NO_THROW(to_debug(vec1, e_debug_style_big));
	EXPECT_NO_THROW(to_debug(vec2, e_debug_style_big));
	EXPECT_NE(to_debug(vec1, e_debug_style_big), to_debug(vec2, e_debug_style_big));
}

TEST(string_utils, debug_simple_hash) {
	EXPECT_NO_THROW(debug_simple_hash(""));
	EXPECT_NO_THROW(debug_simple_hash("sadfsdfa"));
	EXPECT_NO_THROW(debug_simple_hash("123123445"));
	EXPECT_NO_THROW(debug_simple_hash("!@#3e234!RDQWER"));
	EXPECT_NO_THROW(debug_simple_hash(";;l[['l[l;"));
	EXPECT_EQ(debug_simple_hash(""), debug_simple_hash(""));
	EXPECT_NE(debug_simple_hash(""), debug_simple_hash("a"));
	EXPECT_EQ(debug_simple_hash("aaaaa"), debug_simple_hash("aaaaa"));
}

TEST(string_utils, to_binary_string) {
	std::array<char, 4> arr_char {{'a', '&', 'L', '\0'}};
	std::array<char, 4> arr_char2 {{0, 7, 46, 2}};

	EXPECT_NO_THROW(to_binary_string(arr_char));
	EXPECT_NO_THROW(to_binary_string(arr_char2));

	EXPECT_EQ(to_binary_string(arr_char), to_binary_string(arr_char));
	EXPECT_EQ(to_binary_string(arr_char2), to_binary_string(arr_char2));
	EXPECT_NE(to_binary_string(arr_char), to_binary_string(arr_char2));
}

TEST(string_utils, chardbg) {
	EXPECT_NO_THROW(chardbg('a'));
	EXPECT_NO_THROW(chardbg('Z'));
	EXPECT_NO_THROW(chardbg('1'));
	EXPECT_NO_THROW(chardbg('&'));
	EXPECT_NO_THROW(chardbg(';'));
	EXPECT_NO_THROW(chardbg(0));
	EXPECT_NO_THROW(chardbg(127));
	EXPECT_NO_THROW(chardbg(-128));
	EXPECT_EQ(chardbg('q'), std::string("q"));
	EXPECT_EQ(chardbg('a'), std::string("a"));
	EXPECT_EQ(chardbg('G'), std::string("G"));
	EXPECT_EQ(chardbg('7'), std::string("7"));
	EXPECT_EQ(chardbg('('), std::string("("));
	EXPECT_NE(chardbg(4), std::string("4"));
}

TEST(string_utils, ostream_for_boost_any) {
	boost::any any_char = 'a';
	boost::any any_int = 3847;
	boost::any any_bool = true;
	boost::any any_float = 10.542f;
	boost::any any_double = 354.48;
	boost::any any_string = std::string("swufyie");
	boost::any any_vector_string = std::vector<std::string>{"sdfsf", "ru8999999rorg", "&^%^Y&^%"};

	std::ostringstream oss;
	EXPECT_NO_THROW(oss << any_char);
	EXPECT_EQ(oss.str().at(0), boost::any_cast<char>(any_char));

	oss.str("");
	EXPECT_NO_THROW(oss << any_int);
	EXPECT_EQ(oss.str(), std::to_string(boost::any_cast<int>(any_int)));

	oss.str("");
	EXPECT_NO_THROW(oss << any_bool);
	EXPECT_EQ(oss.str(), std::to_string(boost::any_cast<bool>(any_bool)));

	oss.str("");
	EXPECT_NO_THROW(oss << any_float);
	EXPECT_FLOAT_EQ(std::stof(oss.str()), std::stof(std::to_string(boost::any_cast<float>(any_float))));

	oss.str("");
	EXPECT_NO_THROW(oss << any_double);
	EXPECT_DOUBLE_EQ(std::stod(oss.str()), std::stod(std::to_string(boost::any_cast<double>(any_double))));

	oss.str("");
	EXPECT_NO_THROW(oss << any_string);
	EXPECT_EQ(oss.str(), boost::any_cast<std::string>(any_string));

	EXPECT_NO_THROW(oss << any_vector_string);

	oss.str("");
	boost::any any_vector_int = std::vector<int> {1, 5, 7, 3, 0, 123321};
	EXPECT_NO_THROW(oss << any_vector_int);
	EXPECT_EQ(oss.str(), "(no-debug for this boost any type)");
}
