// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "../libs0.hpp"
#include "gtest/gtest.h"
#include "../utils/time_utils.hpp"

using namespace time_utils;

std::string gen_formated_date(int year, int month, int day) {
	std::time_t t = std::time(NULL);
	std::tm tm = *std::localtime(&t);

	// -1900 because year start from 1900
	tm.tm_year = year - 1900;
	// -1 because [0-11]
	tm.tm_mon = month-1;
	tm.tm_mday = day;

	// Daylight Saving Time flag. The value is positive if DST is in effect
	tm.tm_isdst=1;
	//tm.tm_gmtoff = 3600;

	std::time_t test_time = mktime( &tm );
	return timepoint_to_readable(test_time);
}

TEST(time_utils, check_readable_time_length) {

	std::string formated = gen_formated_date(1023,6,1);
	EXPECT_TRUE(formated.length() == 25);

	formated = gen_formated_date(1923,12,30);
	EXPECT_TRUE(formated.length() == 25);

	formated = gen_formated_date(2923,10,20);
	EXPECT_TRUE(formated.length() == 25);
}

TEST(time_utils, check_readable_time_regex) {

	//example match: 2017-01-02T09:10:53+00:00
	const std::string year_regexp(R"((?:[1-9]\d{3})");
	const std::string month_regexp(R"((?:(?:0[1-9]|1[0-2]))");
	const std::string day_regexp(R"((?:0[1-9]|1\d|2[0-8])|(?:0[13-9]|1[0-2])-(?:29|30)|(?:0[13578]|1[02])-31)|(?:[1-9]\d(?:0[48]|[2468][048]|[13579][26])|(?:[2468][048]|[13579][26])00)-02-29))");
	const std::string hour(R"((?:[01]\d|2[0-3]):[0-5]\d:[0-5]\d)");
	const std::string timeshift(R"((?:Z|[+-][01]\d:[0-5]\d))");
	std::regex re("^" + year_regexp + "-" + month_regexp + "-" + day_regexp + "T" + hour + timeshift + "$");
	std::smatch result;


	std::string formated = gen_formated_date(1123,6,1);
	_mark("time: " << formated << ", formated_length: " << formated.length());
	EXPECT_TRUE(std::regex_match(formated, result, re));

	formated = gen_formated_date(1923,12,30);
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = gen_formated_date(2923,10,20);
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = gen_formated_date(2017,4,20);
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());
}

