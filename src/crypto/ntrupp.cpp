// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#if ENABLE_CRYPTO_NTRU


#include "ntrupp.hpp"

#include "../trivialserialize.hpp"

#include "ntru/include/ntru_crypto.h"
#include "ntru/include/ntru_crypto_drbg.h"

// ntru sign
extern "C" {
#include <constants.h>
#include <pass_types.h>
#include <hash.h>
#include <ntt.h>
#include <pass.h>
}


namespace ntrupp {

void NTRU_DRBG_exec_or_throw( uint32_t errcode , const std::string &info="") {
	if (errcode != DRBG_OK)
		throw std::runtime_error(info + " , error code: " + std::to_string(errcode));
}

void NTRU_exec_or_throw( uint32_t errcode , const std::string &info="") {
	if (errcode != NTRU_OK)
		throw std::runtime_error(info + " , error code: " + std::to_string(errcode));
}

uint8_t get_entropy(ENTROPY_CMD cmd, uint8_t *out) {
	static std::ifstream rand_source;
	static sodiumpp::locked_string random_byte(1);

	if (cmd == INIT) {
		/* Any initialization for a real entropy source goes here. */
		rand_source.open("/dev/urandom");
		return 1;
	}

	if (out == nullptr)
		return 0;

	if (cmd == GET_NUM_BYTES_PER_BYTE_OF_ENTROPY) {
		/* Here we return the number of bytes needed from the entropy
		 * source to obtain 8 bits of entropy.  Maximum is 8.
		 */
		*out = 1;
		return 1;
	}

	if (cmd == GET_BYTE_OF_ENTROPY) {
		if (!rand_source.is_open())
			return 0;

		rand_source.get(random_byte[0]);
		*out = static_cast<uint8_t>(random_byte[0]);
		return 1;
	}
	return 0;
}

DRBG_HANDLE get_DRBG(size_t size) {
	// TODO(r) use std::once / lock? - not thread safe now
	static map<size_t , DRBG_HANDLE> drbg_tab;

	auto found = drbg_tab.find(size);
	if (found == drbg_tab.end()) { // not created yet
		try {
			_note("Creating DRBG for size=" << size);
			DRBG_HANDLE newone;
			NTRU_DRBG_exec_or_throw(
						ntru_crypto_drbg_instantiate(size, nullptr, 0, get_entropy, &newone)
						,"random init"
						);
			drbg_tab[ size ] = newone;
			_note("Creating DRBG for size=" << size << " - ready, as drgb handler=" << newone);
			return newone;
		} catch(...) {
			_erro("Can not init DRBG! (exception)");
			throw;
		}
	} // not found
	else {
		return found->second;
	}
	assert(false);
}

std::pair<sodiumpp::locked_string, std::string> generate_encrypt_keypair() {

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	// generate key pair
	uint16_t public_key_len = 0, private_key_len = 0;
	// get size of keys:
	NTRU_exec_or_throw(
				ntru_crypto_ntru_encrypt_keygen(
					get_DRBG(128),
					NTRU_EES439EP1,
					&public_key_len,
					nullptr,
					&private_key_len,
					nullptr)
				,"generate keypair - get key length");

	// values for NTRU_EES439EP1
	assert(public_key_len == 609);
	assert(private_key_len == 659);

	std::string public_key(public_key_len, 0);
	sodiumpp::locked_string private_key(private_key_len);

	NTRU_exec_or_throw(
				ntru_crypto_ntru_encrypt_keygen(get_DRBG(128),
								NTRU_EES439EP1,
								&public_key_len,
								reinterpret_cast<uint8_t *>(&public_key[0]),
								&private_key_len,
								reinterpret_cast<uint8_t *>(private_key.buffer_writable()))
				,"generate keypair");

	ntt_cleanup();

	return std::make_pair(std::move(private_key), std::move(public_key));
}

std::pair<sodiumpp::locked_string, std::string> generate_sign_keypair() {

	sodiumpp::locked_string private_key(PASS_N*sizeof(int64_t));
	std::string public_key(PASS_N*sizeof(int64_t), '\0');

	int64_t * const private_key_ptr = reinterpret_cast<int64_t * const>(&private_key[0]);
	int64_t * const public_key_ptr = reinterpret_cast<int64_t * const>(&public_key[0]);

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	gen_key(private_key_ptr);
	gen_pubkey(public_key_ptr, private_key_ptr);

	std::vector <std::string> public_key_data_vector = {
		std::string("1"), // ntru version
		public_key,
	};
	trivialserialize::generator gen(1);
	gen.push_vector_string(public_key_data_vector);
	ntt_cleanup();

	return std::make_pair(std::move(private_key), std::move(gen.str_move()));
}

std::string sign(const std::string &msg, const sodiumpp::locked_string &private_key) {

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	int64_t z[PASS_N];

	unsigned char hash[HASH_BYTES];
	crypto_hash_sha512(hash,
					   reinterpret_cast<const unsigned char*>(private_key.data()),
					   sizeof(int64_t)*PASS_N); // necessary?

	::sign(hash,
		   z,
		   reinterpret_cast<const int64_t *>(private_key.data()),
		   reinterpret_cast<const unsigned char *>(msg.data()),
		   msg.size());

	std::string signature(reinterpret_cast<const char *>(z), PASS_N * sizeof(int64_t));
	assert(std::memcmp(z, signature.data(), PASS_N * sizeof(int64_t)) == 0);
	// signature = z + hash
	signature.append(reinterpret_cast<char *>(hash), HASH_BYTES);

	ntt_cleanup();
	return signature;
}

bool verify(const std::string &sign, const std::string &msg, const std::string &public_key) {

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), public_key);
	auto public_key_vector_data = parser.pop_vector_string();
	std::string public_key_data = public_key_vector_data.at(1);
	std::string message_hash = sign.substr(PASS_N * sizeof(int64_t));
	auto ret = ::verify(
		reinterpret_cast<const unsigned char *>(message_hash.data()),
		reinterpret_cast<const int64 *>(sign.data()),
		reinterpret_cast<const int64 *>(public_key_data.data()),
		reinterpret_cast<const unsigned char *>(msg.data()),
		msg.size());
	return ret == VALID;
}
} // namspace ntrupp


#endif

