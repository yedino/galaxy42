#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <string>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace time_utils {

	using t_sysclock = std::chrono::system_clock;  ///< clock I will use for my timing
	using t_timepoint = std::chrono::time_point<t_sysclock>; ////< timepoint I will use for my timing

	/**
	 * generate exact date, with optional daylight time saving (isdst)
	 * function uses std::chrono::system_clock::now(), should be used only for test purposes
	 *
	 * final arguments values:
	 * @param year years from 0, could be negative
	 * @param month months – [1, 12]
	 * @param day day of the month – [1, 31]
	 * @param hour hours since midnight – [0, 23]
	 * @param min minutes after the hour – [0, 59]
	 * @param sec seconds after the minute – [0, 61](until C++11) / [0, 60] (since C++11)
	 *
	 * @throw exception for date prior to Jan 1, 1970
	 */
	std::time_t gen_exact_date(int year,
	                           int month,
	                           int day,
	                           int hour,
	                           int min,
	                           int sec,
	                           int isdst = -1);

	/// Convert t_timepoint to readable date in accordance with ISO 8601 - Combined date and time in UTC
	/// Only leaving zone empyt guarantees thread safety and time accuracy
	std::string timepoint_to_readable(const t_timepoint &tp, const std::string &zone = "");

	/// Convert time_t to readable date in accordance with ISO 8601 - Combined date and time in UTC
	/// Only leaving zone empyt guarantees thread safety and time accuracy
	std::string time_t_to_readable(const std::time_t &time, const std::string &zone = "");

	/// get only date, without timezone info
	std::string get_date_str(const std::time_t &time);

	/**
	 * change zone offset format accordinig to ISO 8601:
	 * +0200->+02:00
	 * -0430->-04:00
	 * because of zone setting, gen_exact_date uses C getenv, setenv, tzset function
	 *
	 * This functions is not thread safe !
	 *
	 * available zones could be find on:
	 *   https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
	 * or on linux in file:
	 *   /usr/share/zoneinfo/zone.tab
	 */
	std::string get_zone_utc_offset(const boost::posix_time::ptime& utc_time,
	                                const std::string &zone = "");


	boost::posix_time::time_duration get_utc_offset(const boost::posix_time::ptime &utc_time);

	/// getting default timezone without getenv, setenv, tzset functions
	std::string get_utc_offset_string(const boost::posix_time::ptime& utc_time);
}

#endif // TIME_UTILS_HPP
