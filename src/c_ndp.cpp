#include "c_ndp.hpp"

#if defined(ANTINET_windows)

#include <iostream>
#include <iomanip>
#include <libs0.hpp>
#include <boost/asio.hpp>

static const bool little_edian = [] {
	uint16_t number = 1;
	return ((reinterpret_cast<unsigned char *>(&number))[0] == 1);
}();

// Here we build ethernet frame template (level 2) as in https://en.wikipedia.org/wiki/Ethernet_frame#Ethernet_II
std::array<unsigned char, 94> c_ndp::m_generate_neighbor_advertisement_packet = {
	//=== ethernet header ===//
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // destination MAC will be set later
	0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, // source MAC always the same, only this seems to work; Tested 0xFD 00 ... 00 it does not work
	0x86, 0xDD, 	// EtherType 0x86DD == IPv6 (https://en.wikipedia.org/wiki/EtherType)

	//=== ipv6 header ===//
	0x60, 0x00, 0x00, 0x00, // flow label
	0x00, 0x28, // payload len, TODO current value is from wireshark
	0x3A, // next header 56 == ICMPv6
	0xFF, // hop limit
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // source ipv6 address (filled later in the code using this packet-template)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // destination ipv6 address (filled later in the code using this packet-template)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	//=== icmpv6 ===//
	// Below we have the body of NDP-reply, as in: https://tools.ietf.org/html/rfc4861#section-4.4
	0x88, // type 136 (Neighbor Advertisement) https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol_version_6
	0x00, // code
	0x00, 0x00, // checksum (filled later in the code using this packet-template)
	0xE0, // set flags R, S, O
	0x00, 0x00, 0x00, // reserved
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // target ipv6 address (will be filed later)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x01, // options, works this way, tested in Wireshark; Could be explained more based on RFC4861 "Possible options:" in 4.4
	0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, // source MAC always the same
	// optins described in https://tools.ietf.org/html/rfc4861#section-4.6
	0x01, // type: source link-layer address
	0x01, // length
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // link layer address
};

// size of array must be the same as in c_tun_device_windows::m_buffer 
bool c_ndp::is_packet_neighbor_solicitation(const std::array<uint8_t, 9000> &packet_data) {
	// ethernet header = 14
	// ipv6 header = 40
	// tested in wireshark
	const uint8_t * const packet_type = &packet_data.front() + 14 + 40;

	if (*packet_type == 135) return true;
	return false;
}

std::array<uint8_t, 94> c_ndp::generate_neighbor_advertisement(const std::array<uint8_t, 9000> &neighbor_solicitation_packet) {
	std::array<uint8_t, 94> return_packet;
	const uint8_t * const input_src_mac_address = &neighbor_solicitation_packet.front() + 6;

	//=== ethernet header ===//
	auto it = return_packet.begin(); // points to destination MAC
	for (int i = 0; i < 6; ++i) { // destination MAC
		*it = *(input_src_mac_address + i);
		++it;
	}
	// source MAC
	*it = 0xFC; ++it;
	for (int i = 0; i < 5; ++i) {
		*it = 0x00; ++it;
	}
	// EtherType
	// 0x86DD == IPv6 (https://en.wikipedia.org/wiki/EtherType)
	*it = 0x86; ++it;
	*it = 0xDD; ++it;

	//=== ipv6 header ===//
	// flow label
	*it = 0x60; ++it;
	*it = 0x00; ++it;
	*it = 0x00; ++it;
	*it = 0x00; ++it;
	// payload len
	*it = 0x00; ++it; // TODO current value is from wireshark
	*it = 0x28; ++it;
	// next header
	*it = 0x3A; ++it; // ICMPv6 58
	// hop limit
	*it = 0xFF; ++it;
	//source address
	const uint8_t * const input_icmp_begin = &neighbor_solicitation_packet.front() + 54; // 54 == sum of headers
	const uint8_t * const input_icmp_target_address = input_icmp_begin + 8;
	for (int i = 0; i < 16; ++i) {
		*it = *(input_icmp_target_address + i);
		++it;
	}
	// destination address
	const uint8_t * const input_source_ipv6_address = &neighbor_solicitation_packet.front() + 22;
	for (int i = 0; i < 16; ++i) {
		*it = *(input_source_ipv6_address + i);
		++it;
	}
	//=== icmpv6 ===//
	*it = 136; ++it; // type 136 (Neighbor Advertisement)
	*it = 0; ++it; // code
	// checksum
	auto it_checksum = it;
	*it = 0x00; ++it;
	*it = 0x00; ++it;
	// set flags R, S, O
	*it = 0xE0; ++it;
	// reserved
	*it = 0x00; ++it;
	*it = 0x00; ++it;
	*it = 0x00; ++it;
	// target address
	for (int i = 0; i < 16; ++i) {
		*it = *(input_icmp_target_address + i);
		++it;
	}
	// options
	*it = 0x02; ++it; // type: target link-layer address
	*it = 0x01; ++it; // length
	// source MAC
	*it = 0xFC; ++it;
	for (int i = 0; i < 5; ++i) { // link-layer address
		*it = 0x00; ++it;
	}
	*it = 0x01; ++it; // type: source link-layer address
	*it = 0x01; ++it; // length
	for (int i = 0; i < 6; ++i) { // link-layer address
		*it = *(input_src_mac_address + i);
		++it;
	}
	// calculate checksum
	uint16_t checksum = checksum_ipv6_packet(&return_packet.front() + 22, &return_packet.front() + 54, 40, 58);
	*it_checksum = reinterpret_cast<uint8_t*>(&checksum)[0]; ++it_checksum;
	*it_checksum = reinterpret_cast<uint8_t*>(&checksum)[1]; ++it_checksum;
	return return_packet;
}

uint16_t c_ndp::checksum_ipv6_packet(const uint8_t *source_destination_addr, const uint8_t *header_with_content, uint16_t length, uint32_t next_hvalue) {
	_check_abort(length > 0);
	if(little_edian)
		next_hvalue = htonl(next_hvalue);

	uint64_t result = 0;
	uint8_t sd_addr_size = 32;
	for (uint8_t i = 0; i < sd_addr_size; i += 2) {
		result += source_destination_addr[i];
		result += source_destination_addr[i + 1] << 8;
	}

	for (uint32_t i = 0; i < length; i += 2) {
		result += header_with_content[i];
		result += header_with_content[i + 1] << 8;
	}

	if (length & 1)
		result += little_edian ? (header_with_content[length - 1] << 8) : (header_with_content[length - 1]);

	uint32_t length_bigendian = length;
	if(little_edian)
		length_bigendian = htonl(length);

	result += (length_bigendian >> 16) + (length_bigendian & 0xFFFF);
	result += (next_hvalue >> 16)      + (next_hvalue & 0xFFFF);

	while (result >> 16)
		result = (result >> 16) + (result & 0xFFFF);

	return ~result;
}

uint16_t c_ndp::checksum_ipv6_packet(tab_view<uint8_t> source_destination_addr, tab_view<uint8_t> header_with_content,	uint32_t next_hvalue) {
	uint16_t length = header_with_content.size();
	_check_abort(length > 0);
	if(little_edian)
		next_hvalue = htonl(next_hvalue);

	uint64_t result = 0;
	uint8_t sd_addr_size = 32;
	for (uint8_t i = 0; i < sd_addr_size; i += 2) {
		result += source_destination_addr.at(i);
		result += source_destination_addr.at(i + 1) << 8;
	}

	for (uint32_t i = 0; i < length; i += 2) {
		result += header_with_content.at(i);
		result += header_with_content.at(i + 1) << 8;
	}

	if (length & 1)
		result += little_edian ? (header_with_content.at(length - 1) << 8) : (header_with_content.at(length - 1));

	uint32_t length_bigendian = length;
	if(little_edian)
		length_bigendian = htonl(length);

	result += (length_bigendian >> 16) + (length_bigendian & 0xFFFF);
	result += (next_hvalue >> 16)      + (next_hvalue & 0xFFFF);

	while (result >> 16)
		result = (result >> 16) + (result & 0xFFFF);

	return ~result;
}

#endif // _WIN32
