#include "my_tests_lib.hpp"

#include "../utils/capmodpp.hpp"
#include<sstream>

using namespace capmodpp;

TEST( capmodpp_test , operators_test ) {

	cap_perm yes = cap_perm::yes;
	cap_perm no = cap_perm::no;
	std::ostringstream osstream;
	osstream << yes;
	EXPECT_EQ(osstream.str(), std::string("YES"));
	osstream << no;
	EXPECT_EQ(osstream.str(), std::string("no"));
}
