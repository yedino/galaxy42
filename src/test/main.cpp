// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "my_tests_lib.hpp"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

	::testing::InitGoogleTest(&argc, argv);

	// Gets hold of the event listener list.
	::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
	// Adds a listener to the end.  Google Test takes the ownership.
	listeners.Append(new TestsEnvReseter);

  return RUN_ALL_TESTS();
}

