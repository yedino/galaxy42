#ifndef C_NDP_HPP
#define C_NDP_HPP

#include <platform.hpp>
#if defined(ANTINET_windows)

#include <array>
#include <cstdint>
#include <stdplus/tab.hpp>

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
		// [performance]
		// TODO remove new from name after delete deprecated version
		template <typename T, typename U>
		static const std::array<unsigned char, 94> generate_neighbor_advertisement_new(
			const T & mac_address_container,
			const U & packet_neighbor_solicitation_container);

		// next_hvalue: 58 icmpv6 and 17 for udpv6
		[[deprecated]]
		static uint16_t checksum_ipv6_packet(
				const uint8_t* source_destination_addr,
				const uint8_t* header_with_content,
				uint16_t length,
				uint32_t next_hvalue);

		static uint16_t checksum_ipv6_packet(
				tab_view<uint8_t> source_destination_addr,
				tab_view<uint8_t> header_with_content,
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
	const int offset_icmpv6_type = 14 + 40 + 0; // +0 bytes inside ICMPv6 header to get the type
	if (size <= (offset_icmpv6_type)) return false; // check size of packet
	const unsigned char * const packet_type = data + offset_icmpv6_type;
	if (*packet_type == 135) return true;
	return false;
}

template <typename T, typename U>
const std::array<unsigned char, 94> c_ndp::generate_neighbor_advertisement_new(
	const T & mac_address_container,
	const U & packet_neighbor_solicitation_container) {
		stdplus::copy_safe_apart(6, mac_address_container, m_generate_neighbor_advertisement_packet); // copy 6 bytes of mac address
		stdplus::copy_safe_apart(16, packet_neighbor_solicitation_container, m_generate_neighbor_advertisement_packet, 62, 22); // copy ipv6 address into ipv6.src field
		stdplus::copy_safe_apart(16, packet_neighbor_solicitation_container, m_generate_neighbor_advertisement_packet, 22, 38); // copy ipv6 address into ipv6.dst field
		stdplus::copy_safe_apart(16, packet_neighbor_solicitation_container, m_generate_neighbor_advertisement_packet, 62, 62); // copy ipv6 address into icmpv6.target_address field
		stdplus::copy_safe_apart(6, mac_address_container, m_generate_neighbor_advertisement_packet, 0, 88); // copy 6 bytes of mac address into icmpv6.link_layer address field
		// clear checksum field
		m_generate_neighbor_advertisement_packet.at(56) = 0x00;
		m_generate_neighbor_advertisement_packet.at(57) = 0x00;

		tab_view<uint8_t> source_destination_addr(m_generate_neighbor_advertisement_packet, 22, 22 + 32);
		tab_view<uint8_t> header_with_content(m_generate_neighbor_advertisement_packet, 54, 54 + 40);
		uint16_t checksum = checksum_ipv6_packet(source_destination_addr, header_with_content, 58);
		m_generate_neighbor_advertisement_packet.at(56) = checksum & 0xFF;
		m_generate_neighbor_advertisement_packet.at(57) = checksum >> 8;
		return m_generate_neighbor_advertisement_packet;
}

#endif // ANTINET_windows

#endif
