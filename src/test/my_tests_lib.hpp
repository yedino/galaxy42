// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "gtest/gtest.h"
#include "../c_tnetdbg.hpp"

// https://github.com/google/googletest/blob/master/googletest/include/gtest/gtest.h
class TestsEnvReseter : public ::testing::EmptyTestEventListener {
	public:
	
		long int m_count_tests_skipped;

		TestsEnvReseter();

		// Called before a test starts.
		void OnTestStart(const ::testing::TestInfo& test_info) override;

		// Fired after all test activities have ended.
		void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override;
		
		virtual void TestIsSkipped(const std::string & why);

		static TestsEnvReseter & instance();
};

#define TEST_IS_DISABLED( why ) do { \
		TestsEnvReseter::instance().TestIsSkipped( why  ); \
	} while(0)


