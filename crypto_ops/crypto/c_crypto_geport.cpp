/* See the LICENSE.txt for license information */

#include "c_crypto_geport.hpp"

signed_msg c_crypto_geport::sign (const string &msg, long_type Private_key[256 + 8]) {
	long_type hashed_msg = sha256<long_type>(msg);

	signed_msg signature;

	signature.public_key = 0;
	signature.pop_count = bits_counter(hashed_msg);
	long_type hashed_private_key;


	if (signature.pop_count > 256/2) {
		hashed_msg = ~hashed_msg;
	}

	for (short i = 0; i < 256; ++i) {
		hashed_private_key = sha256<long_type>((string)Private_key[i]);
		if (hashed_msg & (1 << i))
			signature.Signature[i] = Private_key[i];
		else
			signature.Signature[i] = hashed_private_key;

		join_hash(signature.public_key, sha256<long_type>((string)hashed_private_key));
	}

	for (short i = 256; i < 256 + 8; ++i) {
		hashed_private_key = sha256<long_type>((string)Private_key[i]);
		if (signature.pop_count & (1 << i))
			signature.Signature[i] = Private_key[i];
		else
			signature.Signature[i] = hashed_private_key;

		join_hash(signature.public_key, sha256<long_type>((string)hashed_private_key));
	}
	return signature;
}

bool c_crypto_geport::verify_sign (const string &msg, const signed_msg &signature) {
	long_type hashed_msg = sha256<long_type>(msg), tmp;
	long_type key = 0;
	long_type hashed_signature;

	if (signature.pop_count != bits_counter(hashed_msg))
		return false;


	if (signature.pop_count > 256/2) {
		hashed_msg = ~hashed_msg;
	}

	for (short i = 0; i < 256; ++i) {
		hashed_signature = sha256<long_type>((string)signature.Signature[i]);
		if (hashed_msg & (1 << i))
			tmp = hashed_signature;
		else
			tmp = signature.Signature[i];

		join_hash(key, sha256<long_type>((string)tmp));
	}

	for (short i = 256; i < 256 + 8; ++i) {
		hashed_signature = sha256<long_type>((string)signature.Signature[i]);
		if (signature.pop_count & (1 << i))
			tmp = hashed_signature;
		else
			tmp = signature.Signature[i];

		join_hash(key, sha256<long_type>((string)tmp));
	}
	return (key == signature.public_key);
}

void c_crypto_geport::generate_private_key (c_crypto_geport::long_type *Private_key) {
	for (size_t i = 0; i < 256 + 8; ++i)
		Private_key[i] = rd_gen.get_random((256 + 8) / 8);
}

void c_crypto_geport::join_hash (long_type &a, const long_type &b) {
	a <<= 256;
	a += b;
	a = sha256<long_type>((string)a);
}

size_t c_crypto_geport::bits_counter (long_type value) {
	size_t counter = 0;
	for (size_t i = 0; i < 256; ++i, value >>= 1) {
		if (value & 1)
			++counter;
	}
	return counter;
}
