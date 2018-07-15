// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "my_tests_lib.hpp"
#include "utils/unused.hpp"

TestsEnvReseter::TestsEnvReseter() : m_count_tests_skipped(0) { }

std::string banner_skip() {
	//     "[  PASSED  ]"
	return "[---SKIP---]" ;
}


// Called before a test starts.
void TestsEnvReseter::OnTestStart(const ::testing::TestInfo& test_info) {
	pfp_UNUSED(test_info);
	g_dbg_level_set(150, "Be quiet during tests", true, true);
	// test_info.test_case_name(), test_info.name());
}

// Fired after all test activities have ended.
void TestsEnvReseter::OnTestProgramEnd(const ::testing::UnitTest& unit_test) {
	pfp_UNUSED(unit_test);
	g_dbg_level_set(40, "Show again some debug after the tests");

	auto skipped = instance().m_count_tests_skipped;
	if (skipped>0) {
		std::cout << banner_skip() << " Tests skipped: " << skipped
		<< "." << std::endl;
		pfp_warn("Skipped some tests.");
	}
	else std::cout << "(No tests were skipped, this is good)" << std::endl;

	std::cout << std::endl;
}

void TestsEnvReseter::TestIsSkipped(const std::string & why) {
	++ instance().m_count_tests_skipped;
	std::cout << banner_skip() << " " << why << std::endl;
}

TestsEnvReseter & TestsEnvReseter::instance() {
	static TestsEnvReseter obj;
	return obj;
}

