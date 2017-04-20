#ifndef C_NDP_HPP
#define C_NDP_HPP

#include <platform.hpp>
#if defined(ANTINET_windows)

#include <array>
#include <cstdint>

class c_tun_device_windows;

/**
 * @brief The c_ndp class
 * This Class sends NDP (Neighbor Discovery Protocol) packets
 * Neighbor Discovery Protocol is described in rfc4861
 * https://tools.ietf.org/html/rfc4861
 *
 * this will be used when one computer is windows, that is:
 * (Windows galaxy42 peer) -----> (Linux galaxy42 peer)
 *
 * 1) Windows application wants to send packet to fd42::abcd - therefore it will use galaxy42 tuntap
 * 2) Windows kernel first automatically sends an NDP-request packet (to see that there are any peers available through that tuntap - otherwise it would conclude there are 0 devices with ipv6 connected to this tuntap and would drop all packets it seems)
 * 3) Windows gives this NDP-request packet into galaxy42 because it is sent via tuntap, it is an ICMPv6
 * 4) Windows galaxy42 program (we here) see the NDP-request, and we react to it, by sending back to tuntap an NDP-reply
 * 5) Windows kernel sees the NDP-reply, and thinks there are connected ipv6-peers to that tuntap, and it will allow to send IPv6 over this tuntap from now on
 * 6) We (windows galaxy42 program) reply back to ourselves
 */

class c_ndp {
	public:
		[[deprecated]]
		static bool is_packet_neighbor_solicitation
			(const std::array<uint8_t, 9000> &packet_data);

		/**
		 * @param data pointer to buffer contains ETH frame readed from windows TAP
		 * @param size number of bytes pointed by data
		 * @returns true if packet is neighbor slolicitation
		 */
		template <typename T>
		static bool is_packet_neighbor_solicitation(const T * const data, size_t size) noexcept;

		[[deprecated]]
		static std::array<uint8_t, 94> generate_neighbor_advertisement
			(const std::array<uint8_t, 9000> &neighbor_solicitation_packet);

		/**
		 * Generate response packet for neighbor solicitation
		 * @param sol_src_mac pointer to data buffer containst source mac address from neighbor solicitation packet\n
		 * all values between sol_stc_mac and sol_src_mac + 6 must be valid
		 * @param sol_target_address pointer to data buffer contains ipv6 target address from neighbor solicitation packet\n
		 * all values between sol_target_address and sol_target_address + 16 must be valid
		 * @returns neighbor advertisement packet
		 * Thread safe: no (internal static array is modified)
		 */
		template <typename T>
		static const std::array<unsigned char, 94> &generate_neighbor_advertisement(
			const T * const sol_src_mac,
			const T * const source_address_ipv6,
			const T * const destination_address_ipv6) noexcept;

		// next_hvalue: 58 icmpv6 and 17 for udpv6

		static uint16_t checksum_ipv6_packet(
				const uint8_t* source_destination_addr,
				const uint8_t* header_with_content,
				uint16_t length,
				uint32_t next_hvalue);
	private:
		static std::array<unsigned char, 94> m_generate_neighbor_advertisement_packet;

};

template<typename T>
bool c_ndp::is_packet_neighbor_solicitation(const T * const data, size_t size) noexcept {
	static_assert(CHAR_BIT == 8, "");
	static_assert(sizeof(typename std::remove_pointer<decltype(data)>::type) == 1, "");
	// ethernet header = 14
	// ipv6 header = 40
	// tested on wireshark
	if (size < (14 + 40)) return false;
	const unsigned char * const packet_type = data + 14 + 40;
	if (*packet_type == 135) return true;
	return false;
}

template<typename T>
const std::array<unsigned char, 94> &c_ndp::generate_neighbor_advertisement(
	const T * const sol_src_mac,
	const T * const source_address_ipv6,
	const T * const destination_address_ipv6) noexcept {

		static_assert(CHAR_BIT == 8, "");
		static_assert(sizeof(typename std::remove_pointer<decltype(sol_src_mac)>::type) == 1, "");
		static_assert(sizeof(typename std::remove_pointer<decltype(source_address_ipv6)>::type) == 1, "");
		static_assert(sizeof(typename std::remove_pointer<decltype(destination_address_ipv6)>::type) == 1, "");
		std::copy(sol_src_mac, sol_src_mac + 6, &m_generate_neighbor_advertisement_packet.at(0)); // copy 6 bytes of mac address
		std::copy(source_address_ipv6, source_address_ipv6 + 16, &m_generate_neighbor_advertisement_packet.at(22)); // copy ipv6 address into ipv6.src field
		std::copy(destination_address_ipv6, destination_address_ipv6 + 16, &m_generate_neighbor_advertisement_packet.at(38)); // copy ipv6 address into ipv6.dst field
		std::copy(source_address_ipv6, source_address_ipv6 + 16, &m_generate_neighbor_advertisement_packet.at(62)); // copy ipv6 address into icmpv6.target_address field
		std::copy(sol_src_mac, sol_src_mac + 6, &m_generate_neighbor_advertisement_packet.at(88)); // copy 6 bytes of mac address into icmpv6.link_layer address field
		// clear checksum field
		m_generate_neighbor_advertisement_packet.at(56) = 0x00;
		m_generate_neighbor_advertisement_packet.at(57) = 0x00;

		uint16_t checksum = checksum_ipv6_packet(&m_generate_neighbor_advertisement_packet.front() + 22, &m_generate_neighbor_advertisement_packet.front() + 54, 40, 58);
		m_generate_neighbor_advertisement_packet.at(56) = checksum & 0xFF;
		m_generate_neighbor_advertisement_packet.at(57) = checksum >> 8;
		return m_generate_neighbor_advertisement_packet;
}

#endif // ANTINET_windows

#endif
