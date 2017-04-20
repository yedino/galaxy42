#ifndef TIME_UTILS_HPP
#define TIME_UTILS_HPP

#include <string>
#include <chrono>

namespace time_utils {

	using t_sysclock = std::chrono::system_clock;  ///< clock I will use for my timing
	using t_timepoint = std::chrono::time_point<t_sysclock>; ////< timepoint I will use for my timing

	/// convert t_timepoint to readable date in accordance with ISO 8601 - Combined date and time in UTC
	std::string timepoint_to_readable(const t_timepoint &tp);

	/// timepoint_to_readable overload for std::time_t
	std::string timepoint_to_readable(const std::time_t &time);

	/// change zone offset format accordinig to ISO 8601:
	/// +0200->+02:00
	/// -0430->-04:00
	std::string get_zone_iso8601(const time_t &time);

}

#endif // TIME_UTILS_HPP
