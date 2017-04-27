// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "../libs0.hpp"
#include "gtest/gtest.h"
#include "../utils/time_utils.hpp"

using namespace time_utils;

// TODO write unit test that meet "Leap second" exception

TEST(time_utils, check_readable_time_length) {

	std::string formated = time_t_to_readable(gen_exact_date(1023,6,1,1,1,1));
	EXPECT_TRUE(formated.length() == 25);

	formated = time_t_to_readable(gen_exact_date(1923,12,30,2,2,2));
	EXPECT_TRUE(formated.length() == 25);

	formated = time_t_to_readable(gen_exact_date(2923,10,20,3,3,3));
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


	std::string formated = time_t_to_readable(gen_exact_date(1023,6,1,1,1,1));
	_mark("time: " << formated << ", formated_length: " << formated.length());
	EXPECT_TRUE(std::regex_match(formated, result, re));

	formated = time_t_to_readable(gen_exact_date(1923,12,30,2,2,2));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = time_t_to_readable(gen_exact_date(2923,10,20,3,3,3));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());

	formated = time_t_to_readable(gen_exact_date(2017,4,20,5,30,15));
	EXPECT_TRUE(std::regex_match(formated, result, re));
	_mark("time: " << formated << ", formated_length: " << formated.length());
}

TEST(time_utils, t_timepoint_overload) {
	t_timepoint now_time = t_timepoint::clock::now();
	std::string formated = timepoint_to_readable(now_time);
	EXPECT_TRUE(formated.length() == 25);
}

TEST(time_utils, exact_data_check) {

	std::string formated = time_t_to_readable(gen_exact_date(2017,10,21,12,1,3), "Europe/Warsaw");
	_mark("time Europe/Warsaw: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2017-10-21T12:01:03+02:00");

	formated = time_t_to_readable(gen_exact_date(2017,10,1,0,0,0), "Asia/Pyongyang");
	_mark("time Asia/Pyongyang: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2017-10-01T00:00:00+08:30");

	formated = time_t_to_readable(gen_exact_date(2022,12,15,15,0,0), "Pacific/Galapagos");
	_mark("time Pacific/Galapagos: " << formated << ", formated_length: " << formated.length());
	EXPECT_EQ(formated, "2022-12-15T15:00:00-06:00");
}

TEST(time_utils, default_tz) {

	unsetenv("TZ");
	std::string formated = time_t_to_readable(gen_exact_date(2019,11,21,11,10,30));
	EXPECT_TRUE(formated.find("2019-11-21T11:10:30") != std::string::npos);
}
