
#include "platform.hpp"


#ifdef ANTINET_linux

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
		EXPECT_EQ(osstream.str(), std::string("no."));
	}
}

TEST( capmodpp_test , cap_state_operator_test ) {
	cap_state test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=no. permit=no. inherit=no. bound=no."));
	}
	test_obj.bounding=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=no. permit=no. inherit=no. bound=YES"));
	}
	test_obj.eff=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=YES permit=no. inherit=no. bound=YES"));
	}
	test_obj.inherit=cap_perm::yes;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=YES permit=no. inherit=YES bound=YES"));
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
		std::string str = osstream.str();
		EXPECT_NE(str.find("CAP state with 0 CAPs defined:"), std::string::npos);
	}
	test_obj.state = {{0, cap_state()}, {1, cap_state()}, {2, cap_state()} };
	{
		std::ostringstream osstream;
		osstream << test_obj;
		std::string str = osstream.str();
		EXPECT_NE(str.find("CAP state with 3 CAPs defined:"), std::string::npos);
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
		EXPECT_NE(osstream.str().find("remove"), std::string::npos);
	}
}

TEST( capmodpp_test , cap_statechange_operator_test ) {
	cap_statechange test_obj;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		//std::string out = "eff=" + STR(cap_permchange::(same)) + " permit="
		EXPECT_EQ(osstream.str(), std::string("eff=(same) permit=(same) inherit=(same) bound=(same)"));
	}
	test_obj.bounding=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=(same) permit=(same) inherit=(same) bound=ENABLE"));
	}
	test_obj.eff=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=ENABLE permit=(same) inherit=(same) bound=ENABLE"));
	}
	test_obj.inherit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=ENABLE permit=(same) inherit=ENABLE bound=ENABLE"));
	}
	test_obj.permit=cap_permchange::enable;
	{
		std::ostringstream osstream;
		osstream << test_obj;
		EXPECT_EQ(osstream.str(), std::string("eff=ENABLE permit=ENABLE inherit=ENABLE bound=ENABLE"));
	}
}

#endif

