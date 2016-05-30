#include "ntrupp.hpp"

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
	throw std::runtime_error(info + " , error code: " + std::to_string(errcode));
}

void NTRU_exec_or_throw( uint32_t errcode , const std::string &info="") {
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

std::pair<sodiumpp::locked_string, std::string> generate_keypair() {

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
	sodiumpp::locked_string private_key(private_key_len);

	NTRU_exec_or_throw(
				ntru_crypto_ntru_encrypt_keygen(get_DRBG(128), NTRU_EES439EP1,
												&public_key_len, reinterpret_cast<uint8_t *>(&public_key[0]),
				&private_key_len, reinterpret_cast<uint8_t *>(private_key.buffer_writable())
				)
			,"generate keypair"
			);
	unsigned char hash[HASH_BYTES];
	crypto_hash_sha512(hash,
					   reinterpret_cast<const unsigned char*>(private_key.data()),
					   sizeof(int64)*PASS_N); // necessary?

	public_key += std::string(reinterpret_cast<const char *>(hash));


	std::string public_and_hash(public_key + std::string(reinterpret_cast<const char *>(hash)),
								public_key_len + HASH_BYTES);

	assert(public_and_hash.size() == (public_key_len + HASH_BYTES)
		   && "Ntru public_key + hash: BAD sizes" );

	return std::make_pair(std::move(private_key), std::move(public_and_hash));
}

std::string sign(const std::string &msg, const sodiumpp::locked_string &private_key) {

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	int64_t z[PASS_N];

	unsigned char hash[HASH_BYTES];
	crypto_hash_sha512(hash,
					   reinterpret_cast<const unsigned char*>(private_key.data()),
					   sizeof(int64)*PASS_N); // necessary?

	::sign(hash,
		   z,
		   reinterpret_cast<const int64_t *>(private_key.data()),
		   reinterpret_cast<const unsigned char *>(msg.data()),
		   msg.size());

	std::string signature(reinterpret_cast<const char *>(z), PASS_N);

	ntt_cleanup();

	return signature;
}

void verify(const std::string &sign, const std::string &msg, const std::string &public_key) {

	if(ntt_setup() == -1) {
		throw std::runtime_error("ERROR: Could not initialize FFTW. Bad wisdom?");
	}

	//		std::string public_key = pubkey.substr()
	//		::verify()

	//		ntt_cleanup();
}

std::string ntru_encrypt(const sodiumpp::locked_string plain, const std::string &pubkey) {
	uint16_t cyphertext_size=0;

	const auto & drbg = get_DRBG(128);

	// first run just to get the size of output:
	ntru_crypto_ntru_encrypt( drbg,
							  numeric_cast<uint16_t>(pubkey.size()), reinterpret_cast<const uint8_t*>(pubkey.c_str()),
							  numeric_cast<uint16_t>(plain.size()),  reinterpret_cast<const uint8_t*>(plain.c_str()),
							  &cyphertext_size, NULL	);
	assert( (cyphertext_size!=0) || (plain.size()==0) );
	assert( (cyphertext_size >= plain.size()) );

	string ret( cyphertext_size , static_cast<char>(0) ); // allocate memory of the encrypted text
	assert( ret.size() == cyphertext_size );
	// actually encrypt now:
	ntru_crypto_ntru_encrypt( drbg,
							  numeric_cast<uint16_t>(pubkey.size()), reinterpret_cast<const uint8_t*>(pubkey.c_str()),
							  numeric_cast<uint16_t>(plain.size()), reinterpret_cast<const uint8_t*>(plain.c_str()),
							  &cyphertext_size, reinterpret_cast<uint8_t*>(&ret[0])	);

	return ret;
}

sodiumpp::locked_string ntru_decrypt(const std::string cyphertext, const sodiumpp::locked_string &PRVkey) {
	uint16_t cleartext_len=0;
	ntru_crypto_ntru_decrypt(
				numeric_cast<uint16_t>(PRVkey.size()), reinterpret_cast<const uint8_t*>(PRVkey.c_str()),
				numeric_cast<uint16_t>(cyphertext.size()), reinterpret_cast<const uint8_t*>(cyphertext.c_str()),
				&cleartext_len, NULL);
	assert( (cleartext_len!=0) || (cyphertext.size()==0) );

	sodiumpp::locked_string ret( cleartext_len );
	assert( ret.size() == cleartext_len );
	ntru_crypto_ntru_decrypt(
				numeric_cast<uint16_t>(PRVkey.size()), reinterpret_cast<const uint8_t*>(PRVkey.c_str()),
				numeric_cast<uint16_t>(cyphertext.size()), reinterpret_cast<const uint8_t*>(cyphertext.c_str()),
				&cleartext_len, reinterpret_cast<uint8_t*>(ret.buffer_writable()));

	return ret;
}

} // namspace ntrupp
