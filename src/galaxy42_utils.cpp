

#include "galaxy42_utils.hpp"
#include <boost/asio.hpp>

#include "libs0.hpp"

std::pair<std::string, int> tunserver_utils::parse_ip_string(const std::string &ip_string) {
	try {

	if (ip_string.size()<(1+1)*4) _throw_error(runtime_error(join_string_sep("Invalid (too small) IP size",ip_string.size())));
	if (ip_string.size()>(4*4+1+4)) _throw_error(runtime_error(join_string_sep("Invalid (too big) IP size",ip_string.size())));
	/*
	std::regex pattern("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}:\\d{1,5}"); // i.e. 127.0.0.1:4562
	std::smatch result;
	if (!std::regex_search(ip_string, result, pattern)) { // bad argument
//		throw std::invalid_argument("bad format of input ip address");
                throw std::invalid_argument(mo_file_reader::gettext("L_bad_format_of_input_ip_address"));

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
	if (port_int<=0) _throw_error(runtime_error(join_string_sep("Invalid port", port_int)));
	if (port_int>65535) _throw_error(runtime_error(join_string_sep("Invalid port", port_int)));
	_info("Parsing IP as strings: " << ip <<  " port int: " << port_int);

	// tests with boost:
	// validate ipv4
	boost::system::error_code ec;
	auto ip_boost = boost::asio::ip::address_v4::from_string(ip, ec);
	if (ec) _throw_error(runtime_error(join_string_sep("Invalid address (boost detected)", ip, ec)));
	// boost::asio::ip::address_v6::from_string(r_ipv6, ec); // for ipv6

	//auto ip_bytes = ip_boost.to_bytes();
	//assert( ip_bytes.size()==4 );
	bool is_good_class = ip_boost.is_class_a() || ip_boost.is_class_b() || ip_boost.is_class_c()
		|| ip_boost.is_loopback();
	bool is_bad_class = ip_boost.is_unspecified() || ip_boost.is_multicast();

	if (!( is_good_class && !is_bad_class ))
		_throw_error(runtime_error(join_string_sep("Invalid address (not a normal ip-class of address)",ip_boost)));

	return std::make_pair(std::move(ip), port_int);

	} catch(std::exception &ex) {
		_throw_error(std::invalid_argument( join_string_sep( ex.what() , mo_file_reader::gettext("L_bad_format_of_input_ip_address")) ));
	}
}
