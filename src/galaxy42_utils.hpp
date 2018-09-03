
#pragma once

#include <stdplus/misc.hpp>

/// Converts string like "100.200.50.50:32000"
/// @throws std::invalid_argument
std::pair<std::string, int> tunserver_utils::parse_ip_string(const std::string &ip_string);

