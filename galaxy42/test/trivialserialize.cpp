#include "gtest/gtest.h"
#include "../trivialserialize.hpp"

using namespace trivialserialize;

TEST(serialize, uvarint) {
	for (uint64_t i = 1; i < 30000000; i++) {
	//for (uint64_t i = 0; i < 255; i++) {
		generator gen(1);
		gen.push_integer_uvarint(i);
		trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), gen.str());
		ASSERT_EQ(parser.pop_integer_uvarint(), i);
	}
}
