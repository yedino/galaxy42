#include "time_utils.hpp"

#include <iomanip>
#include <ctime>
#include "platform.hpp"
#include "tnetdbg.hpp"

#include <boost/date_time/c_local_time_adjustor.hpp>

std::time_t time_utils::gen_exact_date(int year,
                                       int month,
                                       int day,
                                       int hour,
                                       int min,
                                       int sec,
                                       int isdst) {
	using namespace boost::posix_time;


	// local tm for recognizing Daylight Saving Time
	std::chrono::system_clock::time_point local_timepoint = std::chrono::system_clock::now();
	std::time_t local_time = std::chrono::system_clock::to_time_t(local_timepoint);
	std::tm *local_tm = std::localtime(&local_time);

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

	// isdst less than zero means that the information is not available.
	// setting local dst
	if (isdst == -1) {
		tm.tm_isdst=local_tm->tm_isdst;
	} else {
		// Daylight Saving Time flag. The value is positive if DST is in effect
		tm.tm_isdst=isdst;
	}

	ret = std::mktime(&tm);

	// adding local utc offset in total second to std::mktime.
	time_duration utc_offset = get_utc_offset(ptime_from_tm(*local_tm));
	auto l_seconds = utc_offset.total_seconds();

	ret += l_seconds;

	return ret;
}

std::string time_utils::timepoint_to_readable(const time_utils::t_timepoint &tp, const std::string &zone) {
	std::time_t time = std::chrono::system_clock::to_time_t(tp);
	return time_t_to_readable(time, zone);
}

std::string time_utils::time_t_to_readable(const std::time_t &time, const std::string &zone) {
	using namespace boost::posix_time;

	std::string full_date = get_date_str(time);

	if(zone.empty()) {
		full_date += time_utils::get_utc_offset_string(from_time_t(time));
	} else {
		full_date += time_utils::get_zone_utc_offset(from_time_t(time), zone);
	}

	return full_date;
}

std::string time_utils::get_date_str(const std::time_t &time) {
	using namespace boost::posix_time;
	ptime  t = from_time_t(time);

	return to_iso_extended_string(t) ;
}

boost::posix_time::time_duration time_utils::get_utc_offset(const boost::posix_time::ptime& utc_time) {
	using boost::posix_time::ptime;
	using boost::date_time::c_local_adjustor;

	const ptime local_time = c_local_adjustor<ptime>::utc_to_local(utc_time);
	return local_time - utc_time;
}

std::string time_utils::get_utc_offset_string(const boost::posix_time::ptime& utc_time) {
	std::stringstream out;

	using namespace boost::posix_time;
	time_facet* tf = new time_facet();
	tf->time_duration_format("%+%H:%M");
	out.imbue(std::locale(out.getloc(), tf));

	out << get_utc_offset(utc_time);

	return out.str();
}

std::string time_utils::get_zone_utc_offset(const boost::posix_time::ptime& utc_time,
                                            const std::string &zone) {

	// save actual // char* because getenv could return null
	char* ctz(getenv("TZ"));

	// set variable zone
	auto err = setenv("TZ", zone.c_str(), 1);
	if(err) _warn("Could not set TZ variable, time/date shown may be incorrect");

	tzset();

	std::string zone_str = get_utc_offset_string(utc_time);

	// back to actual
	if(ctz != NULL) {
		setenv("TZ", ctz, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();

	return zone_str;
}

