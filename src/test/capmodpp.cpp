#include "my_tests_lib.hpp"

#include "../utils/capmodpp.hpp"
#include <stdplus/misc.hpp>
#include<sstream>

using namespace capmodpp;
using stdplus::STR;

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

TEST( capmodpp_test , cap_state_map_operator_test ) {
	cap_state_map test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), "");
	}
	test_obj.state = {{0, cap_state()}, {1, cap_state()}, {2, cap_state()} };
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "0: " + STR(cap_state()) + "\n1: " + STR(cap_state()) + "\n2: " + STR(cap_state()) + "\n";
		EXPECT_EQ(osstream.str(), out);
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
		std::string out = "eff=" + STR(cap_permchange::unchanged) + " permit=" + STR(cap_permchange::unchanged)
				+ " inherit=" + STR(cap_permchange::unchanged) + " bound=" + STR(cap_permchange::unchanged);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.bounding=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::unchanged) + " permit=" + STR(cap_permchange::unchanged)
				+ " inherit=" + STR(cap_permchange::unchanged) + " bound=" + STR(cap_permchange::enable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.eff=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::enable) + " permit=" + STR(cap_permchange::unchanged)
				+ " inherit=" + STR(cap_permchange::unchanged) + " bound=" + STR(cap_permchange::enable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.inherit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::enable) + " permit=" + STR(cap_permchange::unchanged)
				+ " inherit=" + STR(cap_permchange::enable) + " bound=" + STR(cap_permchange::enable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.permit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::enable) + " permit=" + STR(cap_permchange::enable)
				+ " inherit=" + STR(cap_permchange::enable) + " bound=" + STR(cap_permchange::enable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.bounding=cap_permchange::disable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::enable) + " permit=" + STR(cap_permchange::enable)
				+ " inherit=" + STR(cap_permchange::enable) + " bound=" + STR(cap_permchange::disable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.eff=cap_permchange::disable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::disable) + " permit=" + STR(cap_permchange::enable)
				+ " inherit=" + STR(cap_permchange::enable) + " bound=" + STR(cap_permchange::disable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.inherit=cap_permchange::disable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::disable) + " permit=" + STR(cap_permchange::enable)
				+ " inherit=" + STR(cap_permchange::disable) + " bound=" + STR(cap_permchange::disable);
		EXPECT_EQ(osstream.str(), out);
	}
	test_obj.permit=cap_permchange::disable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "eff=" + STR(cap_permchange::disable) + " permit=" + STR(cap_permchange::disable)
				+ " inherit=" + STR(cap_permchange::disable) + " bound=" + STR(cap_permchange::disable);
		EXPECT_EQ(osstream.str(), out);
	}
}

TEST( capmodpp_test , cap_statechange_map_operator_test ) {
	cap_statechange_map test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), "");
	}
	test_obj.state = {{0, cap_statechange()}, {1, cap_statechange()}, {2, cap_statechange()} };
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "0: " + STR(cap_statechange()) + "\n1: " + STR(cap_statechange()) + "\n2: " + STR(cap_statechange()) + "\n";
		EXPECT_EQ(osstream.str(), out);
	}
}

TEST( capmodpp_test , cap_statechange_full_operator_test ) {
	cap_statechange_full test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), "\nall_others: " + STR(test_obj.all_others));
	}
	test_obj.given.state = {{0, cap_statechange()}, {1, cap_statechange()}, {2, cap_statechange()} };
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string out = "0: " + STR(cap_statechange()) + "\n1: " + STR(cap_statechange())
				+ "\n2: " + STR(cap_statechange()) + "\n\nall_others: " + STR(test_obj.all_others);
		EXPECT_EQ(osstream.str(), out);
	}
}
