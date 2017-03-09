// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "trivialserialize_test_object.hpp"
#include "../trivialserialize.hpp"
#include <exception>

using namespace trivialserialize;


void uvarint_for_i(uint64_t i) {
	generator gen(1);
	gen.push_integer_uvarint(i);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	ASSERT_EQ(parser.pop_integer_uvarint(), i);
}

TEST(serialize, uvarint) {
	for (uint64_t i = 0; i < 1000; i++) uvarint_for_i(i);
	for (uint64_t i = 0; i < 1000*1000; i+=1000) uvarint_for_i(i);
	for (uint64_t i = 0; i < 1000*1000*1000; i+=1000*1000) uvarint_for_i(i);
	for (uint64_t i = 0; i < 3; ++i) uvarint_for_i(256-1+i);
	for (uint64_t i = 0; i < 3; ++i) uvarint_for_i(65536-1+i);
}

TEST(serialize, single_bytes) {
	const signed char s_chars[] = {'a', 'b', 'c'};
	const unsigned char u_chars[] = {'x', 'y', 'z'};
	generator gen(6);
	for (size_t i = 0; i < 3; i++) {
		gen.push_byte_s(s_chars[i]);
		gen.push_byte_u(u_chars[i]);
	}
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	for (size_t i = 0; i < 3; i++) {
		signed char s = parser.pop_byte_s();
		EXPECT_EQ(s, s_chars[i]);
		unsigned char u = parser.pop_byte_u();
		EXPECT_EQ(u, u_chars[i]);
	}
}

TEST(serialize, varstring_vector) {
	generator gen(1);
	std::vector<std::string> input = {
	std::string("asd"),
	std::string("0"),
	std::string("3.141592653589793"),
	std::string("sdf sdfsdf vvv"),
	std::string("sdg;sfgs;33ggg;4432;4;4323;234;"),
	std::string("!@#$%^&*()<>?_=/*-+,.;:"),
	std::string(R"("""""")")
	//std::string("(\"\"\"\"\")")
	};
	gen.push_vector_string(input);

	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	auto output_vector = parser.pop_vector_string();
	for (size_t i = 0; i < output_vector.size(); ++i) {
		ASSERT_EQ(input.at(i), output_vector.at(i));
	}
}

TEST(serialize, get_bad_type) {
	const unsigned int val = 123;
	generator gen(1);
	gen.push_integer_u<1>(val);
	trivialserialize::parser parser(
		trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	ASSERT_THROW(parser.pop_varstring(), std::exception);
}

TEST(serialize, get_from_empty_string) {
	std::string empty("");
	trivialserialize::parser parser(
		trivialserialize::parser::tag_caller_must_keep_this_string_valid() , empty );
	ASSERT_THROW((parser.pop_integer_u<1, uint8_t>()), std::exception);
}

/*TEST(serialize, test_trivialserialize) {
	std::ostringstream oss;
	EXPECT_NO_THROW(test::test_trivialserialize(oss)); // <---
	auto thestr = oss.str();
	EXPECT_GT( thestr.size() , static_cast<size_t>(1000));
	EXPECT_LT( thestr.size() , static_cast<size_t>(10000));
}*/

TEST(serialize, varstring_map) {
	std::map<std::string, std::string> input;
	input["aaa"] = "bbb";
	input["111"] = "2222222222222222222";
	input["0"] = "zzzzzzzzz";
	input["asdfas"] = "5567";
	input[";"] = "...";
	input[",./234"] = ";433334;43;34;34;2<>;";
	input["           "] = "htfthfft";
	input[R"(		  		)"] = "xyz";
	input["%"] = std::string();
	input[std::string()] = std::string();
	input[std::string(254, 'x')] = std::string(254, 'x');
	generator gen(1);
	gen.push_map_object(input);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	auto output = parser.pop_map_object<std::string, std::string>();
	for (const auto &  pair : input) {
		EXPECT_EQ(pair.second, output.at(pair.first));
	}
	EXPECT_EQ( output , input );
	EXPECT_EQ( output.size() , input.size() );
}

TEST(serialize, empty_vector) {
	generator gen(1);
	std::vector<std::string> input;
	gen.push_vector_string(input);
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	auto output_vector = parser.pop_vector_string();
	ASSERT_TRUE(output_vector.empty());
}

TEST(serialize, empty_element_as_last) {
	generator gen(1);
	std::vector<std::string> input = {
	std::string("aaaa"),
	std::string("bbbb"),
	std::string()
	};
	gen.push_vector_string(input);
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	auto output = parser.pop_vector_string();
	ASSERT_EQ(input.size(), output.size());
	for (size_t i = 0; i < input.size(); ++i) {
		ASSERT_EQ(input.at(i), output.at(i));
	}
}

TEST(serialize, empty_element_as_first) {
	generator gen(1);
	std::vector<std::string> input = {
	std::string(),
	std::string("aaaa"),
	std::string("bbbb")
	};
	gen.push_vector_string(input);
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , gen.str() );
	auto output = parser.pop_vector_string();
	ASSERT_EQ(input.size(), output.size());
	for (size_t i = 0; i < input.size(); ++i) {
		ASSERT_EQ(input.at(i), output.at(i));
	}
}

TEST(serialize, get_generator_buffer) {
	generator gen(1);
	gen.push_varstring("serialized_string");
	const std::string str1 = gen.str();
	const std::string str2 = gen.get_buffer();
	EXPECT_FALSE(gen.str().empty());
	const std::string str3 = gen.str_move();
	EXPECT_EQ(str1, str2);
	EXPECT_EQ(str2, str3);
	EXPECT_TRUE(gen.str().empty());
}

TEST(serialize, create_parser) {
	generator gen(1);
	gen.push_varstring("asdasdasd");
	gen.push_integer_uvarint(32111);
	const std::string gen_str1 = gen.str();
	const std::string gen_str2 = gen.str();
	trivialserialize::parser parser_buffer(trivialserialize::parser::tag_caller_must_keep_this_buffer_valid(), gen_str1.data(), gen_str1.size());
	trivialserialize::parser parser_string(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen_str2);
	EXPECT_EQ(parser_buffer.pop_varstring(), parser_string.pop_varstring());
	EXPECT_EQ(parser_buffer.pop_integer_uvarint(), parser_string.pop_integer_uvarint());
}

TEST(serialize, skip_byte) {
	generator gen(1);
	gen.push_byte_u('a');
	const std::string data = "a1a2a3a4a5a6a7a8a9a0a";
	gen.push_varstring(data);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	EXPECT_THROW(parser.pop_byte_skip('z'), format_error_read_delimiter);
	EXPECT_NO_THROW(parser.pop_byte_skip('a'));
	EXPECT_THROW(parser.pop_byte_skip('a'), format_error_read_delimiter);
	std::string readed_data;
	EXPECT_NO_THROW(readed_data = parser.pop_varstring());
	EXPECT_EQ(readed_data, data);
}

TEST(serialize, skip_bytes_n) {
	generator gen(1);
	gen.push_byte_u('a');
	gen.push_byte_s('b');
	gen.push_byte_u('c');
	gen.push_byte_s('d');
	gen.push_byte_s('e');
	gen.push_byte_u('f');
	const std::string data("dghefilh");
	gen.push_varstring(data);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	EXPECT_NO_THROW(parser.skip_bytes_n(0));
	parser.skip_bytes_n(2);
	EXPECT_EQ(parser.pop_byte_u(), 'c');
	EXPECT_THROW(parser.skip_bytes_n(100), format_error_read);
	EXPECT_NO_THROW(parser.skip_bytes_n(3));
	EXPECT_NO_THROW(parser.pop_varstring());
	EXPECT_NO_THROW(parser.skip_bytes_n(0));
	EXPECT_THROW(parser.skip_bytes_n(1), format_error_read);
}

TEST(serialize, skip_var_string) {
	generator gen(1);
	const std::string data1("aaaaaa");
	const std::string data2("bbbbbb");
	const std::string data3("cccccc");
	const std::string data4("dddddd");
	gen.push_varstring(data1);
	gen.push_varstring(data2);
	gen.push_varstring(data3);
	gen.push_varstring(data4);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	EXPECT_EQ(parser.pop_varstring(), data1);
	EXPECT_NO_THROW(parser.skip_varstring());
	EXPECT_EQ(parser.pop_varstring(), data3);
	EXPECT_NO_THROW(parser.skip_varstring());
	EXPECT_THROW(parser.skip_varstring(), format_error_read);
}

TEST(serialize, ojb_serialize) {
	generator gen(1);
	const std::string data1("aaaaaa");
	const std::string data2("bbbbbb");
	obj_serialize(data1, gen);
	obj_serialize(data2, gen);
	obj_serialize('a', gen);
	obj_serialize('b', gen);
	std::vector<std::string> vec {"asda", "asdasd", "asdaaaaaa"};
	obj_serialize(vec, gen);
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
	EXPECT_EQ(obj_deserialize<std::string>(parser), data1);
	EXPECT_EQ(obj_deserialize<std::string>(parser), data2);
	EXPECT_EQ(obj_deserialize<char>(parser), 'a');
	EXPECT_EQ(obj_deserialize<char>(parser), 'b');
	std::vector<std::string> vec2;
	EXPECT_NO_THROW(vec2 = obj_deserialize<std::vector<std::string>>(parser));
	ASSERT_EQ(vec.size(), vec2.size());
	for (size_t i = 0; i < vec.size(); i++)
		EXPECT_EQ(vec.at(i), vec2.at(i));
}

// ==================================================================


vector<c_tank> get_example_tanks() {
	vector<c_tank> data = {
		{ 150, 60 , "T-64"} ,
		{ 500, 70 , "T-72"} ,
		{ 800, 80 , "T-80"} ,
		{ 2000, 90 , "Shilka"} ,
	};
	return data;
}

map<string, c_tank> get_example_tanks_map_location() {
	map<string, c_tank> data = {
		{ "Moscow", { 150, 60 , "T-64"} } ,
		{ "Puszkin" , { 500, 70 , "T-72"} },
	};
	return data;
}

map<c_tank, string> get_example_tanks_map_captain() {
	map<c_tank, string> data = {
		{ { 150, 60 , "T-64"} , "Pavlov" },
		{ { 900, 80 , "BMP"} , "A. Ramius" },
	};
	return data;
}
// ==================================================================

TEST(serialize, multi_serialize) {

	string f="fooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo";

	trivialserialize::generator gen(50);
	gen.push_byte_u(50);
	gen.push_byte_s(-42);
	gen.push_bytes_n(3, "abc");
	gen.push_bytes_n(3, "xyz");
	gen.push_bytes_n(0, "");
	gen.push_bytes_n(6, "foobar");
	gen.push_integer_u<1>(150UL);
	gen.push_integer_u<2>(+30000UL);
	gen.push_integer_u<8>(+30000UL);
	gen.push_integer_u<4>(+2140000000UL);
	gen.push_integer_u<4>(+4294777777UL);
	gen.push_bytes_sizeoctets<1>("Octets1"+f, 100);
	gen.push_bytes_sizeoctets<2>("Octets2"+f, 100);
	gen.push_bytes_sizeoctets<3>("Octets3"+f, 100);
	gen.push_bytes_sizeoctets<4>("Octets4"+f, 100);
	gen.push_bytes_sizeoctets<1>("", 100);

	vector<uint64_t> test_uvarint1 = {1,42,100,250, 0xFD,     0xFE,      0xFF,  1000,     0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFA};
	// width should be:              {1,1,   1,  1,  1+2,     1+2,        1+2,  1+2,         1+2,        1+4,         1+8};
	// serialization should be:                      FD,0,FD  FD,0,FE FD,0,FF  FD,(1000) FD,FFFF  FE,FFFFFFF  FF,FFFFFFFFA
	test_uvarint1.push_back(0xDEADCAFEBEEF);
	for (auto val : test_uvarint1) gen.push_integer_uvarint(val);

	vector<string> test_varstring = {
		string("Hi."),
		string("Now empty string:"),
		string(""),
		string("Now empty string x5"),
		string(""),
		string(""),
		string(""),
		string(""),
		string(""),
		string("Hello!"),
		string(250,'x'),
		string(255,'y'),
		string(400,'z'),
	};
	for (auto val : test_varstring) gen.push_varstring(val);

	gen.push_vector_string( test_varstring );

	gen.push_vector_object( get_example_tanks() );

	gen.push_map_object( get_example_tanks_map_location() );
	gen.push_map_object( get_example_tanks_map_captain() );


	std::map<std::string, std::string> input_map;
	input_map["aaa"] = "bbb";
	input_map["111"] = "2222222222222222222";
	input_map["0"] = "zzzzzzzzz";
	input_map["asdfas"] = "5567";
	input_map[";"] = "...";
	input_map[",./234"] = ";433334;43;34;34;2<>;";
	input_map["           "] = "htfthfft";
	input_map[R"(		  		)"] = "xyz";
	input_map["%"] = std::string();
	input_map[std::string()] = std::string();
	input_map[std::string(259, 'x')] = std::string(259, 'x');
	gen.push_map_object(input_map);

	// ==============================================

	const string input = gen.str();
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , input );

	EXPECT_EQ(parser.pop_byte_u(), 50);
	EXPECT_EQ(parser.pop_byte_s(), -42);

	EXPECT_EQ(parser.pop_bytes_n(3), "abc");
	EXPECT_EQ(parser.pop_bytes_n(3), "xyz");
	EXPECT_NO_THROW(parser.pop_bytes_n(0));
	EXPECT_EQ(parser.pop_bytes_n(6), "foobar");
{
	unsigned int val = 0;
	val = parser.pop_integer_u<1,unsigned int>();
	EXPECT_EQ(val, 150u);
	val = parser.pop_integer_u<2,unsigned int>();
	EXPECT_EQ(val, 30000u);
	val = parser.pop_integer_u<8,unsigned int>();
	EXPECT_EQ(val, 30000u);
	val = parser.pop_integer_u<4,unsigned int>();
	EXPECT_EQ(val, 2140000000u);
	val = parser.pop_integer_u<4,unsigned int>();
	EXPECT_EQ(val, 4294777777u);
}
	EXPECT_EQ(parser.pop_bytes_sizeoctets<1>(),"Octets1" + f);
	EXPECT_EQ(parser.pop_bytes_sizeoctets<2>(), "Octets2" + f);
	EXPECT_EQ(parser.pop_bytes_sizeoctets<3>(), "Octets3" + f);
	EXPECT_EQ(parser.pop_bytes_sizeoctets<4>(), "Octets4" + f);
	EXPECT_EQ(parser.pop_bytes_sizeoctets<1>(), "");

	for (auto val_expected : test_uvarint1) {
		auto val_given = parser.pop_integer_uvarint();
		EXPECT_EQ( val_given, val_expected );
	}

	for (auto val_expected : test_varstring) {
		auto val_given = parser.pop_varstring();
		EXPECT_EQ( val_given, val_expected );
	}

	auto test_varstring_LOADED = parser.pop_vector_string();
	ASSERT_EQ(test_varstring.size(), test_varstring_LOADED.size());
	for (size_t i = 0; i < test_varstring.size(); i++)
		EXPECT_EQ(test_varstring.at(i), test_varstring_LOADED.at(i));

	auto tanks = parser.pop_vector_object<c_tank>();
	auto example_tanks = get_example_tanks();
	ASSERT_EQ(tanks.size(), example_tanks.size());
	for (size_t i = 0; i < tanks.size(); i++)
		EXPECT_EQ(tanks.at(i), example_tanks.at(i));

	auto tanks_location = parser.pop_map_object<string, c_tank>();
	EXPECT_EQ ( tanks_location, get_example_tanks_map_location());

	auto tanks_captain = parser.pop_map_object<c_tank,string>();
	EXPECT_EQ(tanks_captain, get_example_tanks_map_captain());

	auto output = parser.pop_map_object<std::string, std::string>();
	for (const auto &it : output) {
		EXPECT_NO_THROW(input_map.at(it.first));
		EXPECT_EQ(it.second, input_map.at(it.first));
	}
}
