// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "../project.hpp"

#if ! ENABLE_CRYPTO_NTRU
	#error "Do not include this header unless you define ENABLE_CRYPTO_NTRU to true. (Maybe you wrongly configured your project, or maybe you are missing such #if condition before your #include)
#endif

#ifndef NTRUCPP_HPP
#define NTRUCPP_HPP

#include "../libs0.hpp"
#include "sodiumpp/locked_string.h"

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

	uint8_t get_entropy(ENTROPY_CMD cmd, uint8_t *out);
	DRBG_HANDLE get_DRBG(size_t size);

	/// @return pair of <private key, hash_sha512(private_key) + pubkey>
	/// pricate_key hash before publickey is necessary for verifying signatures
	std::pair<sodiumpp::locked_string,std::string> generate_sign_keypair();

	std::string sign(const std::string &msg,
					 const sodiumpp::locked_string &private_key);

	bool verify(const std::string &signature,
				const std::string &msg,
				const std::string &public_key);

	std::pair<sodiumpp::locked_string,std::string> generate_encrypt_keypair();
/*
NTRUCALL
ntru_crypto_ntru_encrypt(
	DRBG_HANDLE     drbg_handle,     //     in - handle for DRBG
	uint16_t        pubkey_blob_len, //     in - no. of octets in public key blob
	uint8_t const  *pubkey_blob,     //     in - pointer to public key
	uint16_t        pt_len,          //     in - no. of octets in plaintext
	uint8_t const  *pt,              //     in - pointer to plaintext
	uint16_t       *ct_len,          // in/out - no. of octets in ct, addr for no. of octets in ciphertext
	uint8_t        *ct);             //    out - address for ciphertext
*/

/***
 * Encrypt plain text for given pubkey.
 */
template<class T>
std::string encrypt(const T &plain, const std::string & pubkey) {
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

/*
NTRUCALL
ntru_crypto_ntru_decrypt(
	uint16_t       privkey_blob_len, //     in - no. of octets in private key
												 blob
	uint8_t const *privkey_blob,     //     in - pointer to private key
	uint16_t       ct_len,           //     in - no. of octets in ciphertext
	uint8_t const *ct,               //     in - pointer to ciphertext
	uint16_t      *pt_len,           // in/out - no. of octets in pt, addr for
												 no. of octets in plaintext
	uint8_t       *pt);              //    out - address for plaintext
*/
//sodiumpp::locked_string decrypt(const string cyphertext, const sodiumpp::locked_string & PRVkey);
template<class T>
T decrypt(const std::string &cyphertext, const sodiumpp::locked_string &PRVkey) {
	uint16_t cleartext_len=0;
	ntru_crypto_ntru_decrypt(
				numeric_cast<uint16_t>(PRVkey.size()), reinterpret_cast<const uint8_t*>(PRVkey.c_str()),
				numeric_cast<uint16_t>(cyphertext.size()), reinterpret_cast<const uint8_t*>(cyphertext.c_str()),
				&cleartext_len, NULL);
	assert( (cleartext_len!=0) || (cyphertext.size()==0) );

	T ret( cleartext_len, '\0');
	assert( ret.size() == cleartext_len );
	ntru_crypto_ntru_decrypt(
				numeric_cast<uint16_t>(PRVkey.size()), reinterpret_cast<const uint8_t*>(PRVkey.c_str()),
				numeric_cast<uint16_t>(cyphertext.size()), reinterpret_cast<const uint8_t*>(cyphertext.c_str()),
				&cleartext_len, reinterpret_cast<uint8_t*>(&ret[0]));

	return ret;
}

}	// namespace ntrucpp

#endif // NTRUCPP_HPP
