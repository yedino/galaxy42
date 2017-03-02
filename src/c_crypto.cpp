#include "c_crypto.hpp"

/// @file this is a TEST-ONLY class for now (main program uses other code)
/// This is based on draft from <hbadger>

c_crypto::c_crypto()
{
	if (sodium_init() == -1) {
		throw std::runtime_error("Fail to init sodium");
	}
	m_my_key = generate_kyepair();
}

cryptobox_keypair c_crypto::generate_kyepair() {
	cryptobox_keypair keys;
	crypto_box_keypair(keys.public_key.data(),
					keys.secret_key.data());
	return keys;
}

const std::array<unsigned char, crypto_box_PUBLICKEYBYTES>& c_crypto::get_my_public_key() {
	return m_my_key.public_key;
}

unsigned char* c_crypto::cryptobox_encrypt(unsigned char *msg, size_t msg_size,
									const std::array<unsigned char, crypto_box_NONCEBYTES> &nonce,
									const std::array<unsigned char, crypto_box_PUBLICKEYBYTES> &public_key) const {

	if (crypto_box_easy(msg, msg, msg_size,
						nonce.data(),
						public_key.data(),
						m_my_key.secret_key.data()) != 0) {
		throw std::runtime_error{"Fail to auth encrypt msg"};
	}
	return msg;
}

unsigned char* c_crypto::cryptobox_decrypt (unsigned char* cipher, size_t cipher_size,
									const std::array<unsigned char, crypto_box_NONCEBYTES> &nonce,
									const std::array<unsigned char, crypto_box_PUBLICKEYBYTES> &public_key) const {
	if (crypto_box_open_easy(cipher, cipher, cipher_size,
							nonce.data(),
							public_key.data(),
							m_my_key.secret_key.data()) != 0) {
		throw std::runtime_error{"Fail to auth decrypt msg"};
	}
	return cipher;
}
