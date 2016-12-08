

#include "galaxy42_utils.hpp"

std::pair<std::string, int> tunserver_utils::parse_ip_string(const std::string &ip_string) {
	if (ip_string.size()<(1+1)*4) _throw_error(runtime_error(join_string_sep("Invalid (too small) IP size",ip_string.size())));
	if (ip_string.size()>(4*4+1+4)) _throw_error(runtime_error(join_string_sep("Invalid (too big) IP size",ip_string.size())));
	/*
	std::regex pattern("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}"); // i.e. 127.0.0.1:4562
	std::smatch result;
	if (!std::regex_search(ip_string, result, pattern)) { // bad argument
//		throw std::invalid_argument("bad format of input ip address");
                throw std::invalid_argument(boost::locale::gettext("L_bad_format_of_input_ip_address"));

	}
	*/
	size_t pos = ip_string.find(':');
	if (pos == string::npos) _throw_error( runtime_error( join_string_sep("Invalid IP format (char ':')", ip_string)));
	std::string ip = ip_string.substr(0, pos);
	if (ip.size()<(1+1)*4) _throw_error(runtime_error(join_string_sep("Invalid (too small) IP size (ip part)",ip.size())));
	std::string port = ip_string.substr(pos+1);
	if (ip.size()<1) _throw_error(runtime_error(join_string_sep("Invalid (too small) IP size (port part)", port.size())));
	_dbg1("Parsing IP as strings: " << ip <<  " port string: " << port);
	int port_int = std::stoi(port);
	if (port_int<0) _throw_error(runtime_error(join_string_sep("Invalid port", port_int)));
	if (port_int>65535) _throw_error(runtime_error(join_string_sep("Invalid port", port_int)));
	_info("Parsing IP as strings: " << ip <<  " port int: " << port_int);
	return std::make_pair(std::move(ip), port_int);
}

