/**

GPL or other licence, see the licence file!

This is early pre-pre-alpha version, do NOT use it yet for anything,
do NOT run it other then in a test VM or better on isolated computer,
it has bugs and 'typpos'.
                                                 _       _
 _ __  _ __ ___       _ __  _ __ ___        __ _| |_ __ | |__   __ _
| '_ \| '__/ _ \_____| '_ \| '__/ _ \_____ / _` | | '_ \| '_ \ / _` |
| |_) | | |  __/_____| |_) | | |  __/_____| (_| | | |_) | | | | (_| |
| .__/|_|  \___|     | .__/|_|  \___|      \__,_|_| .__/|_| |_|\__,_|
|_|                  |_|                          |_|                
     _                       _                                    _   
  __| | ___      _ __   ___ | |_     _   _ ___  ___    _   _  ___| |_ 
 / _` |/ _ \    | '_ \ / _ \| __|   | | | / __|/ _ \  | | | |/ _ \ __|
| (_| | (_) |   | | | | (_) | |_    | |_| \__ \  __/  | |_| |  __/ |_ 
 \__,_|\___/    |_| |_|\___/ \__|    \__,_|___/\___|   \__, |\___|\__|
                                                       |___/          
 _                   _                     
| |__   __ _ ___    | |__  _   _  __ _ ___ 
| '_ \ / _` / __|   | '_ \| | | |/ _` / __|
| | | | (_| \__ \   | |_) | |_| | (_| \__ \
|_| |_|\__,_|___/   |_.__/ \__,_|\__, |___/
                                 |___/  

*/




#pragma once
#ifndef include_crypto_basic_hpp
#define include_crypto_basic_hpp

#include "../libs1.hpp"
#include <sodium.h>
#include "../strings_utils.hpp"
#include "gtest/gtest_prod.h"
#include "ntru/include/ntru_crypto_drbg.h"
#include <sodiumpp/sodiumpp.h>
#include <SIDH.h>




/// @ingroup antinet_crypto
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace antinet_crypto {


std::string to_debug_locked_maybe(const sodiumpp::locked_string & data);

std::string to_debug_locked_maybe(const std::string & data);

sodiumpp::locked_string substr(const sodiumpp::locked_string & str , size_t len);



// extra implementation tools
bool safe_string_cmp(const std::string & a, const std::string & b);


// === for debug ===
std::string to_debug_locked(const sodiumpp::locked_string & data);


// random functions
DRBG_HANDLE get_DRBG(size_t size);

uint8_t get_entropy(ENTROPY_CMD cmd, uint8_t *out);
/**
 * Generate "nbytes" random bytes and output the result to random_array
 * Returns CRYPTO_SUCCESS (=1) on success, CRYPTO_ERROR (=0) otherwise.
 */
CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array);

namespace unittest {
	class c_symhash_state__tests_with_private_access;
} // namespace


// Group: crypto utils, free functions etc

/**
 * @defgroup crypto_utils Crypto utils
 * @ingroup antinet_crypto
 * Usefull free crypto functions
 */
///@{

namespace string_binary_op {

typedef sodiumpp::locked_string LS;
typedef std::string NS;


template <class T1, class T2>
T1 binary_string_xor(T1 const & str1, T2 const& str2) {
	// WARNING: this function is written with less assertive code (e.g. without at()),
	// it MUST be checked against any errors if you would modify it.
	const auto size1 = str1.size();
	const auto size2 = str2.size();
	if (size1 != size2) throw std::runtime_error(
		string("Can not execute function ")	+ string(__func__) + string(" because different size: ")
		+ std::to_string(size1) + " vs " + std::to_string(size2) );

	// this is safe also for locked string:
	T1 ret( str1 );
	for (size_t i=0; i<size1; ++i) ret[i] ^= str2[i];
	// TODO: decltype(size1) without const

	assert(ret.size() == str1.size());	assert(str1.size() == str2.size());
	return ret;
}

std::string operator^(const std::string & str1, const std::string & str2);
sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const sodiumpp::locked_string & str2);
sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const std::string & str2_un);

} //namespace

///@}

// ==================================================================

/**
 * @defgroup hash_func Hashing functions
 * @ingroup antinet_crypto
 * Hashing functions group
 */
///@{
typedef std::string t_hash; ///< type of hash
typedef sodiumpp::locked_string t_hash_PRV; ///< type of hash that contains a private secure data (e.g. is memlocked)

t_hash Hash1( const t_hash & hash );
size_t Hash1_size(); ///< returns size (in octets) of the output of Hash1 function
t_hash Hash2( const t_hash & hash );
size_t Hash2_size(); ///< returns size (in octets) of the output of Hash1 function

t_hash_PRV Hash1_PRV( const t_hash_PRV & hash );
t_hash_PRV Hash2_PRV( const t_hash_PRV & hash );

///@}

// ==================================================================

// must match: t_crypto_system_type_to_name()
enum t_crypto_system_type : unsigned char {
	// 0 is reserved
	e_crypto_system_type_invalid = 0,
	e_crypto_system_type_X25519 = 5,
	e_crypto_system_type_Ed25519 = 6,
	e_crypto_system_type_NTRU_EES439EP1 = 7,
	e_crypto_system_type_SIDH = 8,
	e_crypto_system_type_geport_todo = 9,
	e_crypto_system_type_symhash_todo = 10,
	e_crypto_system_type_NTRU_sign = 11,
	e_crypto_system_type_END_normal,

	e_crypto_system_type_multikey_pub = 29,
	e_crypto_system_type_multikey_private = 30,
	e_crypto_system_type_multisign = 31,
	e_crypto_system_type_END,
};

std::string t_crypto_system_type_to_name(int val);
std::string enum_name(t_crypto_system_type e);

char t_crypto_system_type_to_ID(int val);
t_crypto_system_type t_crypto_system_type_from_ID(char name);
bool t_crypto_system_type_is_asymkex(t_crypto_system_type sys); ///< is this type doing asymmetric KEX instead of D-H

enum t_crypto_use : unsigned char {
	e_crypto_use_secret='S', // for secret key (PRIVATE key)
	e_crypto_use_open='o',  // for open key (public key)
	e_crypto_use_signature='n',  // for signature - the signature data (not a key actually)
	e_crypto_use_fingerprint='f',  // for fingerprinting - e.g. hashes of individual public keys
};

/// A type to count how may keys we have of given crypto system.
typedef std::array< int , e_crypto_system_type_END	> t_crypto_system_count;

// ------------------------------------------------------------------

/** Some state of some crypto system */
class c_crypto_system {
	public:
		typedef sodiumpp::locked_string t_symkey; //< type of symmetric key
		typedef sodiumpp::locked_string t_PRVkey; //< type of private key

		typedef std::string t_pubkey; //< type of public key

		typedef sodiumpp::nonce64 t_crypto_nonce; //< the type of nonce that we use

		virtual ~c_crypto_system()=default;

		// virtual t_symkey secure_random(size_t size_of_radom_data) const;

		virtual t_crypto_system_type get_system_type() const;
};

// ==================================================================



} // namespace

#endif



