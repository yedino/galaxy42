#include "time_utils.hpp"

#include <iomanip>
#include <ctime>
#include "platform.hpp"
#include "tnetdbg.hpp"

std::time_t time_utils::gen_exact_date(int year,
                                                    int month,
                                                    int day,
                                                    int hour,
                                                    int min,
                                                    int sec,
                                                    int isdst) {
	using namespace boost::posix_time;

	std::chrono::system_clock::time_point local_timepoint = std::chrono::system_clock::now();
	std::time_t local_time = std::chrono::system_clock::to_time_t(local_timepoint);

	// we could use duration cast from boost::posix_time::time_duration
	std::istringstream iss(time_utils::get_utc_offset_string());
	std::tm *local_tm = std::localtime(&local_time);

	// parsion string in ISO format to hour and minutes variables example -12:30 -> -12 hours, 30 minutes
	short int l_hours, l_minutes; char ch;
	iss >> l_hours >> ch >> l_minutes;

	time_t ret;
	std::tm tm;

	// -1900 because tm::tm_year starts from 1900
	tm.tm_year = year - 1900;
	// -1 because tm::tm_mon is in range [0-11]
	tm.tm_mon = month -1;
	tm.tm_mday = day;

	tm.tm_hour = hour + l_hours;
	tm.tm_min = min + l_minutes;
	tm.tm_sec = sec;

	// isdst less than zero means that the information is not available.
	// setting local dst
	if (isdst == -1) {
		tm.tm_isdst=local_tm->tm_isdst;
	} else {
		// Daylight Saving Time flag. The value is positive if DST is in effect
		tm.tm_isdst=isdst;
	}

	auto offset = time_utils::get_utc_offset();

	ret = std::mktime(&tm);;

	ptime ptime_from_tm(tm);
	return ret;
}

std::string time_utils::timepoint_to_readable(const time_utils::t_timepoint &tp, const std::string &zone) {
	std::time_t time = std::chrono::system_clock::to_time_t(tp);
	return time_t_to_readable(time, zone);
}

std::string time_utils::time_t_to_readable(const std::time_t &time, const std::string &zone) {

	// Pacific/Marquesas and America/St_Johns exception
	// I have no idea why this two zones do not works properly.
	if(zone == "Pacific/Marquesas" || zone == "America/St_Johns") {
		_warn("Time for " << zone << " zone could not works properly!");
	}

	std::string full_date = get_date_str(time);

	if(zone.empty()) {
		full_date += time_utils::get_utc_offset_string();
	} else {
		full_date += time_utils::get_zone_utc_offset(zone);
	}

	return full_date;
}

std::string time_utils::get_date_str(const std::time_t &time) {
	using namespace boost::posix_time;
	ptime  t = from_time_t(time);

	return to_iso_extended_string(t) ;
}

boost::posix_time::time_duration time_utils::get_utc_offset() {
	using namespace boost::posix_time;

	const ptime utc_now = second_clock::universal_time();
	const ptime now = second_clock::local_time();

	return now - utc_now;
}

std::string time_utils::get_utc_offset_string() {
	std::stringstream out;

	using namespace boost::posix_time;
	time_facet* tf = new time_facet();
	tf->time_duration_format("%+%H:%M");
	out.imbue(std::locale(out.getloc(), tf));

	out << get_utc_offset();

	return out.str();
}

std::string time_utils::get_zone_utc_offset(const std::string &zone) {

	// save actual // char* because getenv could return null
	char* ctz(getenv("TZ"));

	// set variable zone
	if(zone.empty()) {
		setenv("TZ", "", 1);
	} else {
		setenv("TZ", zone.c_str(), 1);
	}
	tzset();

	std::string zone_str = get_utc_offset_string();

	// back to actual
	if(ctz != NULL) {
		setenv("TZ", ctz, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();

	return zone_str;
}
