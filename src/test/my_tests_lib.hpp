// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "gtest/gtest.h"
#include "../tnetdbg.hpp"

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

// -------------------------------------------------------------------
/// TODO move to a lib?
// http://stackoverflow.com/questions/1198260/iterate-over-tuple
// run a function for each element of tuple
template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
	for_each(std::tuple<Tp...> &, FuncT) // Unused arguments are given no names.
	{ }

template<std::size_t I = 0, typename FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
	for_each(std::tuple<Tp...>& t, FuncT f)
	{
		f(std::get<I>(t));
		for_each<I + 1, FuncT, Tp...>(t, f);
	}
// -------------------------------------------------------------------

#define TEST_IS_DISABLED( why ) do { \
		TestsEnvReseter::instance().TestIsSkipped( why  ); \
	} while(0)


