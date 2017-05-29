
#include "ipv6.hpp"
#include <stdplus/tab.hpp>

unsigned int ipv6_size_payload_from_header(stdplus::tab_view<unsigned char> tab) {
	// https://en.wikipedia.org/wiki/IPv6_packet#Fixed_header
	auto v_high = static_cast<unsigned int>(tab.at(4));
	auto v_low  = static_cast<unsigned int>(tab.at(5));
	auto val = v_low + (v_high<<8);
	return val;
}

unsigned int ipv6_size_entireip_from_header(stdplus::tab_view<unsigned char> tab) {
	return ipv6_size_payload_from_header(tab) + 40; // The fixed header of an IPv6 packet consists of its first 40 octets
}


