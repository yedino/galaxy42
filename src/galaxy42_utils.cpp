

#include "galaxy42_utils.hpp"

std::pair<std::string, int> tunserver_utils::parse_ip_string(const std::string &ip_string) {
	/*
	std::regex pattern("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}"); // i.e. 127.0.0.1:4562
	std::smatch result;
	if (!std::regex_search(ip_string, result, pattern)) { // bad argument
//		throw std::invalid_argument("bad format of input ip address");
                throw std::invalid_argument(boost::locale::gettext("L_bad_format_of_input_ip_address"));

	}
	*/
	size_t pos = ip_string.find(':');
	if (pos == string::npos) _throw_error( runtime_error( join_string_sep("Invalid IP format, missing ':'", "pos=",pos, " in [",ip_string,"]")));


	std::string ip = ip_string.substr(0, pos);
	if (pos ) _throw_error( runtime_error( join_string_sep("Invalid IP format, missing ':'", "pos=",pos, " in [",ip_string,"]")));
	std::string port = ip_string.substr(pos+1);
	std::cout << "IP: " << ip <<  " port: " << std::stoi(port) << std::endl;
	return std::make_pair(std::move(ip), std::stoi(port));
}

