#ifndef INCLUDED_api_crypto_fake
#define INCLUDED_api_crypto_fake

#include "libs1.hpp"

#include "api_crypto_base.hpp"

/**
@file Obviously this implements a FAKED cryptography that has 0 security of any sort do not use this for anything other then test/development
*/

namespace nCrypto { 

class c_data_fake : public c_data {
	public:
		virtual std::string serialize() const;
		virtual void deserialize(const std::string & serialized) const;

		virtual bool operator==(const c_data & other) const; ///< comparsions
		virtual bool operator<(const c_data & other) const; ///< for sorting
};

class c_encrypted_data_fake : public c_encrypted_data {
};

class c_plaintext_data_fake : public c_plaintext_data {
};

class c_pubkey_fake : public c_pubkey {
};

class c_privkey_fake : public c_privkey {
};

class c_keypair_fake : public c_pubkey_fake, c_privkey_fake {
};

class c_signature_fake : public c_data {
};

class c_hash_fake : public c_data {
};

class c_api_encryption_fake : public c_api_encryption {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_encrypted_data> encrypt_to(const c_pubkey & recipient, const c_plaintext_data &ptext);
		virtual unique_ptr<c_plaintext_data> decrypt_with(const c_privkey & mykey, const c_encrypted_data &ctext);

		virtual unique_ptr<c_keypair> generate_keypair(); 
};

class c_api_signature_fake : public c_api_signature {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_signature> sign_with(const c_privkey & mykey, const c_plaintext_data &ptext);
		virtual bool check_sign(const c_pubkey & pubkey, const c_signature &signature);
};

class c_api_hash_fake : public c_api_hash {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_hash> hash(const c_data & pubdata);
};


void test_crypto_operations_fake();




} // namespace nCrypto



#endif // include guard

