#ifndef NTRUCPP_HPP
#define NTRUCPP_HPP

#include "libs0.hpp"
#include "sodiumpp/locked_string.h"

// ntru sign
extern "C" {
#include <constants.h>
#include <pass_types.h>
#include <hash.h>
#include <ntt.h>
#include <pass.h>
}

namespace ntrupp {

	/// @return pair of <private key, public_key + hash_sha512(private_key) >
	/// pricate_key hash at the end of publickey is necessary for verifying signatures
	std::pair<sodiumpp::locked_string,std::string> generate_keypair() {

		// generate key pair
		uint16_t public_key_len = 0, private_key_len = 0;
		// get size of keys:
		NTRU_exec_or_throw(
			ntru_crypto_ntru_encrypt_keygen(
				get_DRBG(128),
				NTRU_EES439EP1,
				&public_key_len, nullptr, &private_key_len, nullptr
				)
			,"generate keypair - get key length"
		);
		// values for NTRU_EES439EP1
		assert(public_key_len == 609);
		assert(private_key_len == 659);

		std::string public_key(public_key_len, 0);
		locked_string private_key(private_key_len);

		NTRU_exec_or_throw(
			ntru_crypto_ntru_encrypt_keygen(get_DRBG(128), NTRU_EES439EP1,
				&public_key_len, reinterpret_cast<uint8_t*>(&public_key[0]),
				&private_key_len, reinterpret_cast<uint8_t*>(private_key.buffer_writable())
			)
			,"generate keypair"
		);
		unsigned char hash[HASH_BYTES];
		crypto_hash_sha512(hash,
						   reinterpret_cast<const unsigned char*>(private_key.get_string()),
						   sizeof(int64)*PASS_N); // necessary?

		public_key += std::string(reinterpret_cast<const char>(hash));

		assert(public_key.size() == (public_key_len + HASH_BYTES)
			   && "Ntru public_key + hash: BAD sizes" );


		return std::make_pair(std::move(private_key), std::move(public_key));
	}

	std::string sign(const std::string &msg,
					 const sodiumpp::locked_string &private_key) {

		if(ntt_setup() == -1) {
			throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
		}

		int64_t z[PASS_N];

		unsigned char hash[HASH_BYTES];
		crypto_hash_sha512(hash,
						   reinterpret_cast<const unsigned char*>(private_key.get_string()),
						   sizeof(int64)*PASS_N); // necessary?

		::sign(hash,
			   z,
			   private_key.get_string(),
			   reinterpret_cast<const unsigned char *>(msg.data()),
			   msg.size());

		std::string signature(reinterpret_cast<const char *>(z));

		ntt_cleanup();

		return signature;
	}

	void verify(const std::string &sign,
				const std::string &msg,
				const std::string &pubkey) {

		if(ntt_setup() == -1) {
			throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
		}

		::verify()


		ntt_cleanup();
	}

}	// namespace ntrucpp

#endif // NTRUCPP_HPP
