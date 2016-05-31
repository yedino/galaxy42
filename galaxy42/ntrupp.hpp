#ifndef NTRUCPP_HPP
#define NTRUCPP_HPP

#include "libs0.hpp"
#include "sodiumpp/locked_string.h"


namespace ntrupp {

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
std::string ntru_encrypt(const sodiumpp::locked_string plain, const std::string & pubkey);

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
sodiumpp::locked_string ntru_decrypt(const string cyphertext, const sodiumpp::locked_string & PRVkey);


}	// namespace ntrucpp

#endif // NTRUCPP_HPP
