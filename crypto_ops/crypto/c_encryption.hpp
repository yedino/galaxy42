#ifndef C_ENCRYPTION_HPP
#define C_ENCRYPTION_HPP

#include "c_crypto_RSA.hpp"

#include "ed25519_src/ed25519.h"
#include "ed25519_src/ge.h"
#include "ed25519_src/sc.h"

#include <memory>
#include <cstdlib>

using std::unique_ptr;

constexpr unsigned int key_size = 512;
typedef enum {
	no_crypt, RSA, ed25519
} cryptosign_method;

class c_encryption {
  public:
	c_encryption () : m_crypto_method(no_crypt) { }

	virtual std::string get_public_key () = 0;
	virtual std::string sign (const std::string &msg) = 0;
	virtual bool verify(const std::string &signature, const std::string &message, const std::string &public_key) = 0;

	virtual ~c_encryption () = default;

	cryptosign_method getCrypto_method () {
		return m_crypto_method;
	}
  protected:
	cryptosign_method m_crypto_method;
};

////////////////////////////////////////////////  RSA  /////////////////////////////////////////////////////////////
class c_RSA : public c_encryption {
  public:
	template <typename T>
	struct rsa_public_key {
		rsa_public_key(std::string str) {
			std::string delimeter = "|";
			std::size_t found = str.find(delimeter);
			if (found != std::string::npos) {
				//std::cout << "e: " << str.substr(0,found) << std::endl;	//dbg
				//std::cout << "n: " << str.substr(found+1) << std::endl;	//dbg
				e = long_type(str.substr(0,found));
				n = long_type(str.substr(found+1));
			}
		}
		T e;
		T n;
	};
	typedef number<cpp_int_backend<key_size * 4, key_size * 4, signed_magnitude, unchecked, void>> long_type;

	c_RSA ();

	std::string get_public_key ();
	std::string sign (const std::string &msg);
	bool verify(const std::string &signature, const std::string &message, const std::string &public_key);

  private:
	std::unique_ptr<c_crypto_RSA<key_size>> m_crypto;
	public_key<long_type> m_pub_key;
	long_type m_last_sign;
};

//////////////////////////////////////////////  ED25519  ///////////////////////////////////////////////////////////
typedef enum {
	seed_size = 32, pub_key_size = 32, prv_key_size = 64, sign_size = 64
} ed25519_sizes;


unique_ptr<unsigned char[]> string_to_uniqueUtab(const std::string &str);
std::string uchar_toReadable(const unsigned char* utab, ed25519_sizes size);
unique_ptr<unsigned char[]> readable_toUchar(const std::string &str, ed25519_sizes size);

/**
 * C++ class based on orlp/ed25orlp/ed25519 implemantation
 */
class c_ed25519 : public c_encryption {
  public:
	c_ed25519 ();

	std::string get_public_key (); ///< C++ sign way
	std::string sign (const std::string &msg);

	bool verify (const std::string &signature, const std::string &message, const std::string &public_key);

	unique_ptr<unsigned char[]> get_public_key_uC ();
	unique_ptr<unsigned char[]> sign_uC (unique_ptr<const unsigned char[]> message, size_t message_len);
	bool verify_uC (unique_ptr<const unsigned char[]> &signature,
				   unique_ptr<const unsigned char[]> &message,
				   size_t message_len,
				   unique_ptr<const unsigned char[]> &public_key);

	unsigned char* get_public_key_C();
	unsigned char* sign_C(const unsigned char *message, size_t message_len);	///< faster C sign way
	int verify_C(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key);
	//void add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar);			//
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

