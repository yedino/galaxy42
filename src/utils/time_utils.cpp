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

	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;

	// Daylight Saving Time flag. The value is positive if DST is in effect
	tm.tm_isdst=isdst;
	ret = timegm(&tm);
	return ret;
}

std::string time_utils::timepoint_to_readable(const time_utils::t_timepoint &tp, const std::string &zone) {
	std::time_t time = std::chrono::system_clock::to_time_t(tp);
	return time_t_to_readable(time, zone);
}

std::string time_utils::time_t_to_readable(const std::time_t &time, const std::string &zone) {

	std::string full_date = get_date_str(time);

	if(zone.empty()) {
		full_date += time_utils::get_zone_offset_local(time);
	} else {
		full_date += time_utils::get_zone_offset_universal(time, zone);
	}

	return full_date;
}

// use of std::strftime instead of std::put_time, because of no implementation in gcc version < 5 (4.9.2 on debian8)
std::string time_utils::get_date_str(const std::time_t &time) {
	char buff[35];
	std::strftime(buff, sizeof(buff), "%FT%T", std::gmtime(&time));

	return std::string(buff);
}

// use of std::strftime instead of std::put_time, because of no implementation in gcc version < 5 (4.9.2 on debian8)
std::string time_utils::get_zone_offset_local(const std::time_t &time) {

	char buff[15];
	std::strftime(buff, sizeof(buff), "%z", std::localtime(&time));
	std::string zone_str(buff);

	zone_str.insert(zone_str.end()-2,':');
	return zone_str;
}

std::string time_utils::get_zone_offset_universal(const std::time_t &time, const std::string &zone) {

	// save actual // char* because getenv could return null
	char* ctz(getenv("TZ"));

	// set variable zone
	if(zone.empty()) {
		setenv("TZ", "", 1);
	} else {
		setenv("TZ", zone.c_str(), 1);
	}
	tzset();

	std::string zone_str = get_zone_offset_local(time);

	// back to actual
	if(ctz != NULL) {
		setenv("TZ", ctz, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();

	return zone_str;
}
