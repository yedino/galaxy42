/* See the LICENSE.txt for license information */

#include "c_crypto_ed25519.hpp"

void c_crypto_ed25519::generate_key () {
	ed25519_create_seed(seed);
	ed25519_create_keypair(public_key, private_key, seed);
}

void c_crypto_ed25519::sign (const string &msg, unsigned char *signature) const {
	ed25519_sign(signature, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), public_key, private_key);
}

bool c_crypto_ed25519::verify_sign (const string &msg, const unsigned char *signature, const unsigned char *public_key) const {
	return ed25519_verify(signature, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), public_key) != 0;
}

const unsigned char *const c_crypto_ed25519::get_public_key () const {
	return public_key;
}
