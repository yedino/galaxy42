// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../trivialserialize.hpp"
#include <exception>

using namespace trivialserialize;

TEST(serialize, uvarint) {
	for (uint64_t i = 1; i < 10000000; i++) {
		generator gen(1);
		gen.push_integer_uvarint(i);
		trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
		ASSERT_EQ(parser.pop_integer_uvarint(), i);
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
	std::string(R"(""""")")
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

TEST(serialize, test_trivialserialize) {
	EXPECT_NO_THROW(test::test_trivialserialize());
}

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
