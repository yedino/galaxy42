#include "time_utils.hpp"

#include <iomanip>
#include <ctime>

std::string time_utils::timepoint_to_readable(const time_utils::t_timepoint &tp) {

	std::time_t time = std::chrono::system_clock::to_time_t(tp);
	std::stringstream ss;
	ss << std::put_time(std::gmtime(&time), "%FT%T");

	ss << time_utils::get_zone_iso8601(time);

	return ss.str();
}

std::string time_utils::get_zone_iso8601(const std::time_t &time) {
	std::stringstream zone;
	zone << std::put_time(std::gmtime(&time), "%z");

	std::string zone_str = zone.str();
	zone_str.insert(zone_str.end()-2,':');
	return zone_str;
}
