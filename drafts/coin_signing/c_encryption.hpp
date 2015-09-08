#ifndef C_ENCRYPTION_HPP
#define C_ENCRYPTION_HPP

#include "../../crypto_ops/crypto/ed25519_src/ed25519.h"
#include "../../crypto_ops/crypto/c_crypto_RSA.hpp"

#include "../../crypto_ops/crypto/ed25519_src/ge.h"
#include "../../crypto_ops/crypto/ed25519_src/sc.h"

#include <memory>
#include <cstdlib>

constexpr unsigned int key_size = 512;
typedef enum {
		no_crypt, RSA, ed25519
} cryptosign_method;

class c_encryption {
public:
		c_encryption () : m_crypto_method(no_crypt) { }

		virtual std::string get_public_key () = 0;
		virtual std::string sign (const std::string &msg) = 0;

		virtual ~c_encryption () = default;

        cryptosign_method getCrypto_method () { return m_crypto_method; }
protected:
		cryptosign_method m_crypto_method;
};

class c_RSA : public c_encryption {
public:
		c_RSA ();

		std::string get_public_key ();

		std::string sign (const std::string &msg);

private:
		std::unique_ptr<c_crypto_RSA<key_size>> m_crypto;
};

////////////////////////////////////////////////ED25519/////////////////////////////////////////////////////////////
typedef enum {
		seed_size = 32, pub_key_size = 32, prv_key_size = 64, sign_size = 64
} ed25519_sizes;

std::string uchar_toReadable(unsigned char* utab, ed25519_sizes size);
unsigned char* readable_toUchar(const std::string &str);

/**
 * C++ class based on orlp/ed25orlp/ed25519 implemantation
 */
class c_ed25519 : public c_encryption {
public:
		c_ed25519 ();

		std::string get_public_key (); ///< C++ sign way
		std::string sign (const std::string &msg);

		int verify (const std::string signature, const std::string message, size_t message_len, std::string public_key);

		unsigned char *get_public_key_C ();

		unsigned char *sign_C (const unsigned char *message, size_t message_len);  ///< faster C sign way
		int verify_C (const unsigned char *signature,
			const unsigned char *message,
			size_t message_len,
			const unsigned char *public_key);
		//void add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar);
		//void key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key);

		int create_seed ();

private:
		unsigned char m_signature[sign_size];

		void create_keypair ();

		unsigned char m_public_key[pub_key_size], m_private_key[prv_key_size], m_seed[seed_size];
		//unsigned char m_scalar[32];
		//unsigned char m_other_public_key[32], m_other_private_key[64];
		//unsigned char m_shared_secret[32], m_other_shared_secret[32];
};

#endif // C_ENCRYPTION_HPP
