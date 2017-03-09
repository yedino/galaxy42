#ifndef C_CRYPTO_HPP
#define C_CRYPTO_HPP
#include <sodium.h>
#include "libs0.hpp"

/// @file this is a TEST-ONLY class for now (main program uses other code)
/// This is based on draft from <hbadger>

struct cryptobox_keypair {
	std::array<unsigned char, crypto_box_PUBLICKEYBYTES> public_key;
	std::array<unsigned char, crypto_box_SECRETKEYBYTES> secret_key;
};

class c_crypto
{
	public:
		c_crypto();
		cryptobox_keypair generate_kyepair();
		const std::array<unsigned char, crypto_box_PUBLICKEYBYTES>& get_my_public_key();
		unsigned char* cryptobox_encrypt(unsigned char *msg, size_t msg_size,
									const std::array<unsigned char, crypto_box_NONCEBYTES> &nonce,
									const std::array<unsigned char, crypto_box_PUBLICKEYBYTES> &public_key) const;
		unsigned char* cryptobox_decrypt (unsigned char* cipher, size_t cipher_size,
									const std::array<unsigned char, crypto_box_NONCEBYTES> &nonce,
									const std::array<unsigned char, crypto_box_PUBLICKEYBYTES> &public_key) const;
	private:
		cryptobox_keypair m_my_key;

};

#endif // C_CRYPTO_HPP
