// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"

#include "../c_tnetdbg.hpp"

class TestsEnvReseter : public ::testing::EmptyTestEventListener {
	// Called before a test starts.
	virtual void OnTestStart(const ::testing::TestInfo& test_info) {
		g_dbg_level_set(200, "Be quiet during tests");
		// test_info.test_case_name(), test_info.name());
	}

	// Called after a test ends.
	virtual void OnTestEnd(const ::testing::TestInfo& test_info) { }
};


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

	::testing::InitGoogleTest(&argc, argv);

	// Gets hold of the event listener list.
	::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
	// Adds a listener to the end.  Google Test takes the ownership.
	listeners.Append(new TestsEnvReseter);

	g_dbg_level_set(200, "Be quiet during tests");

  return RUN_ALL_TESTS();
}
