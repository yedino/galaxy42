#include "c_peering.hpp"

#include <sodium.h>

c_peering::c_peering(const c_ip46_addr & addr, const std::string & pubkey)
 : m_addr(addr), m_pubkey(pubkey)
{
	_info("I am new peer, with addr="<<addr<<" and pubkey="<<pubkey);
}



c_peering_udp::c_peering_udp(const c_ip46_addr & addr, const std::string & pubkey)
	: c_peering(addr, pubkey)
{ }

void c_peering_udp::send_data(const char * data, size_t data_size) {
	throw std::runtime_error("Use send_data_udp");
}

void c_peering_udp::send_data_udp(const char * data, size_t data_size, int udp_socket) {
	static unsigned char generated_shared_key[crypto_generichash_BYTES] = {43, 124, 179, 100, 186, 41, 101, 94, 81, 131, 17,
					198, 11, 53, 71, 210, 232, 187, 135, 116, 6, 195, 175,
					233, 194, 218, 13, 180, 63, 64, 3, 11};

	static unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES] = {148, 231, 240, 47, 172, 96, 246, 79};
	static unsigned char additional_data[] = {1, 2, 3};
	static unsigned long long additional_data_len = 3;

	// TODO randomize this data

	std::unique_ptr<unsigned char[]> ciphertext(new unsigned char[data_size + crypto_aead_chacha20poly1305_ABYTES]);
	unsigned long long ciphertext_len;

	assert(crypto_aead_chacha20poly1305_KEYBYTES <= crypto_generichash_BYTES);

	crypto_aead_chacha20poly1305_encrypt(ciphertext.get(), &ciphertext_len, (unsigned char *)data, data_size, additional_data,
	                                     additional_data_len, NULL, nonce, generated_shared_key);

	_info("UDP send to peer: " << data_size << " bytes: [" << string(data,data_size)<<"]" );

	switch (m_addr.get_ip_type()) {
		case c_ip46_addr::t_tag::tag_ipv4 : {
			auto ip_x = m_addr.get_ip4(); // ip of proper type, as local variable
			sendto(udp_socket, ciphertext.get(), ciphertext_len, 0, reinterpret_cast<sockaddr*>( & ip_x ) , sizeof(sockaddr_in) );
		}
		break;
		case c_ip46_addr::t_tag::tag_ipv6 : {
			auto ip_x = m_addr.get_ip6(); // ip of proper type, as local variable
			sendto(udp_socket, ciphertext.get(), ciphertext_len, 0, reinterpret_cast<sockaddr*>( & ip_x ) , sizeof(sockaddr_in6) );
		}
		break;
		default: {
			std::ostringstream oss; oss << m_addr; // TODO
			throw std::runtime_error(string("Invalid IP type: ") + oss.str());
		}
	}
}
