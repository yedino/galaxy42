
#include "platform.hpp"

#ifdef ANTINET_linux

#include "my_tests_lib.hpp"

#include "../utils/capmodpp.hpp"
#include<sstream>

using namespace capmodpp;

TEST( capmodpp_test , cap_perm_operator_test ) {
	cap_perm yes = cap_perm::yes;
	cap_perm no = cap_perm::no;
	{
		std::ostringstream osstream;
		osstream << yes;
		EXPECT_EQ(osstream.str(), std::string("YES"));
	}
	{
		std::ostringstream osstream;
		osstream << no;
		EXPECT_EQ(osstream.str(), std::string("no"));
	}
}

TEST( capmodpp_test , cap_state_operator_test ) {
	cap_state test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=no permit=no inherit=no bound=no"));
	}
	test_obj.bounding=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=no permit=no inherit=no bound=YES"));
	}
	test_obj.eff=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=YES permit=no inherit=no bound=YES"));
	}
	test_obj.inherit=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=YES permit=no inherit=YES bound=YES"));
	}
	test_obj.permit=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=YES permit=YES inherit=YES bound=YES"));
	}
}

TEST( capmodpp_test , cap_permchange_operator_test ) {
	cap_permchange unchanged = cap_permchange::unchanged;
	cap_permchange enable = cap_permchange::enable;
	cap_permchange disable = cap_permchange::disable;
	{
		std::ostringstream osstream;
		osstream << unchanged;
		EXPECT_NE(osstream.str().find("same"), std::string::npos);
	}
	{
		std::ostringstream osstream;
		osstream << enable;
		EXPECT_NE(osstream.str().find("ENABLE"), std::string::npos);
	}
	{
		std::ostringstream osstream;
		osstream << disable;
		EXPECT_NE(osstream.str().find("disable"), std::string::npos);
	}
}

TEST( capmodpp_test , cap_statechange_operator_test ) {
	cap_statechange test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		//std::string out = "eff=" + STR(cap_permchange::unchanged) + " permit="
		EXPECT_EQ(osstream.str(), std::string("eff=unchanged permit=unchanged inherit=unchanged bound=unchanged"));
	}
	test_obj.bounding=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=unchanged permit=unchanged inherit=unchanged bound=enable"));
	}
	test_obj.eff=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=enable permit=unchanged inherit=unchanged bound=enable"));
	}
	test_obj.inherit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=enable permit=unchanged inherit=enable bound=enable"));
	}
	test_obj.permit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=enable permit=enable inherit=enable bound=enable"));
	}
}

#else

TEST( capmodpp_test , nothing_to_test_on_this_platform ) {
}

#endif

