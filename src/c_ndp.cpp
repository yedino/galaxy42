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

	//std::cout << "packet type " << std::dec << std::setw(2) << std::setfill('0') << static_cast<int>(*packet_type) << std::endl;

	if (*packet_type == 135) return true;
	return false;
}

std::array<uint8_t, 9000> c_ndp::generate_neighbor_advertisement (const std::array<uint8_t, 9000> &neighbor_solicitation_packet) {
	std::array<uint8_t, 9000> return_packet;
	const uint8_t const * input_src_mac_address = &neighbor_solicitation_packet.front() + 6;

	// ethernet header
	auto it = return_packet.begin(); // points to destination MAC
	for (int i = 0; i < 6; ++i) { // destination MAC
		*it = *(input_src_mac_address + i);
		++it;
	}
	for (int i = 0; i < 3; ++i) { // source MAC
		*it = 0xFD; ++it;
		*it = 0x42; ++it;
	}
	// EtherType
	// 0x86DD == IPv6 (https://en.wikipedia.org/wiki/EtherType)
	*it = 0x86; ++it;
	*it = 0xDD; ++it;

	// ipv6 header
	// flow label
	*it = 0x60; ++it;
	*it = 0x00; ++it;
	// payload len
	*it = 0x00; ++it; // TODO current value is from wireshark
	*it = 0x28; ++it;
	// next header
	*it = 0x3A; ++it; // ICMPv6 58
	// hop limit
	*it = 0xFF; ++it;
	//source address
	const uint8_t const * input_icmp_begin = &neighbor_solicitation_packet.front() + 54; // 54 == sum of headers
	const uint8_t const * input_icmp_target_address = input_icmp_begin + 8;
	for (int i = 0; i < 16; ++i) {
		*it = *(input_icmp_target_address + i);
		++it;
	}

	return return_packet;
}

#endif // _WIN32
