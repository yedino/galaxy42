#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <string>
#include <chrono>

namespace time_utils {

	using t_sysclock = std::chrono::system_clock;  ///< clock I will use for my timing
	using t_timepoint = std::chrono::time_point<t_sysclock>; ////< timepoint I will use for my timing

	/**
	 * generate exact date, with optional daylight time saving (isdst)
	 * final arguments values:
	 * @param year years from 0, could be negative
	 * @param month months – [1, 12]
	 * @param day day of the month – [1, 31]
	 * @param hour hours since midnight – [0, 23]
	 * @param min minutes after the hour – [0, 59]
	 * @param sec seconds after the minute – [0, 61](until C++11) / [0, 60] (since C++11)
	 */
	time_t gen_exact_date(int year,
	                      int month,
	                      int day,
	                      int hour,
	                      int min,
	                      int sec,
	                      int isdst = 0);

	/// convert t_timepoint to readable date in accordance with ISO 8601 - Combined date and time in UTC
	std::string timepoint_to_readable(const t_timepoint &tp, const std::string &zone = "");

	/// convert time_t to readable date in accordance with ISO 8601 - Combined date and time in UTC
	std::string time_t_to_readable(const std::time_t &time, const std::string &zone = "");

	/**
	 * change zone offset format accordinig to ISO 8601:
	 * +0200->+02:00
	 * -0430->-04:00
	 * because of zone setting, gen_exact_date uses C getenv, setenv, tzset function
	 * that are not thread safe
	 *
	 * available zones could be find on:
	 *   https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
	 * or on linux in file:
	 *   /usr/share/zoneinfo/zone.tab
	 */
	std::string get_zone_offset(const time_t &time, const std::string &zone = "");

}

#endif // TIME_UTILS_HPP
