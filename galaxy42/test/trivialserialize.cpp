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
