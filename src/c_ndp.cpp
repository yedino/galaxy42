#include "c_ndp.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)

#include <iostream>
#include <iomanip>

// size of array must be the same as in c_tun_device_windows::m_buffer 
bool c_ndp::is_packet_neighbor_solicitation(const std::array<uint8_t, 9000> &packet_data) {
/*	const uint8_t const * source_arrdess = &packet_data.front() + 22;
	const uint8_t const * destination_arrdess = &packet_data.front() + 38;

	std::cout << "src address ";
	const uint8_t *it = source_arrdess;
	for (int i = 0; i < 16; ++i) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') <<  static_cast<int>(*it);
		++it;
	}
	std::cout << std::endl;

	std::cout << "dst address ";
	it = destination_arrdess;
	for (int i = 0; i < 16; ++i) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
		++it;
	}
	std::cout << std::endl;
	*/
	// ethernet header = 14
	// ipv6 header = 40
	// tested od wireshark
	const uint8_t const * packet_type = &packet_data.front() + 14 + 40;
	const uint8_t const * code = packet_type + 1;

	//std::cout << "packet type " << std::dec << std::setw(2) << std::setfill('0') << static_cast<int>(*packet_type) << std::endl;

	if (*packet_type == 135) return true;
	return false;
}

#endif
