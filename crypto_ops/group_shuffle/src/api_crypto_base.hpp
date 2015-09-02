#ifndef INCLUDED_api_crypto_base
#define INCLUDED_api_crypto_base

#include "libs1.hpp"

namespace nCrypto { 


class c_data {
	public:
		virtual std::string serialize() const;
		virtual void deserialize(const std::string & serialized) const;

		virtual bool operator==(const c_data & other) const; ///< comparsions
		virtual bool operator<(const c_data & other) const; ///< for sorting
};


class c_encrypted_data : public c_data {
};

class c_plaintext_data : public c_data {
};

class c_pubkey : public c_data {
};

class c_privkey : public c_data {
};

class c_keypair : public c_pubkey, c_privkey {
};

class c_signature : public c_data {
};

class c_hash : public c_data {
};

class c_api_encryption {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_encrypted_data> encrypt_to(const c_pubkey & recipient, const c_plaintext_data &ptext);
		virtual unique_ptr<c_plaintext_data> decrypt_with(const c_privkey & mykey, const c_encrypted_data &ctext);

		virtual unique_ptr<c_keypair> generate_keypair(); 
};

class c_api_signature {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_signature> sign_with(const c_privkey & mykey, const c_plaintext_data &ptext);
		virtual bool check_sign(const c_pubkey & pubkey, const c_signature &signature);
};

class c_api_hash {
	public:
		// use this e.g. on Singleton
		virtual unique_ptr<c_hash> hash(const c_data & pubdata);
};


void test_crypto_operations();




} // namespace nCrypto


#endif // include guard

