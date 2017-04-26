#include "time_utils.hpp"

#include <iomanip>
#include <ctime>
#include "../platform.hpp"

time_t time_utils::gen_exact_date(int year,
                      int month,
                      int day,
                      int hour,
                      int min,
                      int sec,
                      int isdst) {

	time_t ret;
	std::tm tm;

	// -1900 because tm::tm_year starts from 1900
	tm.tm_year = year - 1900;
	// -1 because tm::tm_mon is in range [0-11]
	tm.tm_mon = month -1;
	tm.tm_mday = day;

	// hour is shifted by 1
	tm.tm_hour = hour + 1;
	tm.tm_min = min;
	tm.tm_sec = sec;

	// Daylight Saving Time flag. The value is positive if DST is in effect
	tm.tm_isdst=isdst;
	ret = mktime(&tm);
	return ret;
}

std::string time_utils::timepoint_to_readable(const time_utils::t_timepoint &tp, const std::string &zone) {
	std::time_t time = std::chrono::system_clock::to_time_t(tp);
	return time_t_to_readable(time, zone);
}

std::string time_utils::time_t_to_readable(const std::time_t &time, const std::string &zone) {
	std::stringstream ss;
	ss << std::put_time(std::gmtime(&time), "%FT%T");

	ss << time_utils::get_zone_offset(time, zone);

	return ss.str();
}

std::string time_utils::get_zone_offset(const std::time_t &time, const std::string &zone) {

	// save actual // char* because getenv could return null
	char* ctz(getenv("TZ"));

	// set variable zone
	if(zone.empty()) {
		setenv("TZ", "", 1);
	} else {
		setenv("TZ", zone.c_str(), 1);
	}
	tzset();

	std::stringstream zone_stream;
	zone_stream << std::put_time(std::localtime(&time), "%z");

	// back to actual
	if(ctz != NULL) {
		setenv("TZ", ctz, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();

	std::string zone_str = zone_stream.str();
	zone_str.insert(zone_str.end()-2,':');
	return zone_str;
}
