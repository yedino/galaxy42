#ifndef C_NDP_HPP
#define C_NDP_HPP


#if defined(_WIN32) || defined(__CYGWIN__)

#include <array>
#include <cstdint>

struct Headers_IP6Header {
	uint16_t versionClassAndFlowLabel;

	/** Big Endian. */
	uint16_t flowLabelLow_be;

	/** Big Endian. */
	uint16_t payloadLength_be;

	uint8_t nextHeader;
	uint8_t hopLimit;
	uint8_t sourceAddr[16];
	uint8_t destinationAddr[16];
};

class c_tun_device_windows;

class c_ndp {
	public:
		static bool is_packet_neighbor_solicitation(const std::array<uint8_t, 9000> &packet_data);
};

#endif // _WIN32

#endif