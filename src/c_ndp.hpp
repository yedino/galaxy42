// order32.h
// http://stackoverflow.com/questions/2100331/c-macro-definition-to-determine-big-endian-or-little-endian-machine

#ifndef ORDER32_H
#define ORDER32_H

#include <climits>
#include <cstdint>

#if CHAR_BIT != 8
#error "unsupported char size"
#endif

enum {
	O32_LITTLE_ENDIAN = 0x03020100ul,
	O32_BIG_ENDIAN = 0x00010203ul,
	O32_PDP_ENDIAN = 0x01000302ul
};

static const union { unsigned char bytes[4]; uint32_t value; } o32_host_order = { { 0, 1, 2, 3 } };

#define O32_HOST_ORDER (o32_host_order.value)

#endif


#ifndef C_NDP_HPP
#define C_NDP_HPP


#if defined(_WIN32) || defined(__CYGWIN__)

#include <array>
#include <cstdint>

class c_tun_device_windows;

class c_ndp {
	public:
		static bool is_packet_neighbor_solicitation
			(const std::array<uint8_t, 9000> &packet_data);
		static std::array<uint8_t, 94> generate_neighbor_advertisement
			(const std::array<uint8_t, 9000> &neighbor_solicitation_packet);

		// next_hvalue: 58 icmpv6 and 17 for udpv6

		static uint16_t checksum_ipv6_packet(
				const uint8_t* source_destination_addr,
				const uint8_t* header_with_content,
				uint16_t length,
				uint32_t next_hvalue);

};

#endif // _WIN32

#endif
