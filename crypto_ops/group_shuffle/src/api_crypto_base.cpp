#include "api_crypto_base.hpp"

namespace nCrypto { 



std::string c_data::serialize() const {
	return "";
}

void c_data::deserialize(const std::string & serialized) const {
}

bool c_data::operator==(const c_data & other) const {
	return 0;
}

bool c_data::operator<(const c_data & other) const { 
	return 0;
}


unique_ptr<c_encrypted_data> c_api_encryption::encrypt_to(const c_pubkey & recipient, const c_plaintext_data &ptext) {
	auto r = make_unique<c_encrypted_data>();
	return r;
}

unique_ptr<c_plaintext_data> c_api_encryption::decrypt_with(const c_privkey & mykey, const c_encrypted_data &ctext) {
	auto r = make_unique<c_plaintext_data>();
	return r;
}

unique_ptr<c_keypair> c_api_encryption::generate_keypair() {
	auto r = make_unique<c_keypair>();
	return r;
}


unique_ptr<c_signature> c_api_signature::sign_with(const c_privkey & mykey, const c_plaintext_data &ptext) {
	auto r = make_unique<c_signature>();
	return r;
}

bool c_api_signature::check_sign(const c_pubkey & pubkey, const c_signature &signature) {
	return 0;
}

unique_ptr<c_hash> c_api_hash::hash(const c_data & pubdata) {
	auto r = make_unique<c_hash>();
	return r;
}


} // namespace nCrypto



void nCrypto::test_crypto_operations() {
	using namespace nCrypto;

	c_api_encryption encryption;
	unique_ptr<c_keypair> keypair_alice = encryption.generate_keypair();
	unique_ptr<c_keypair> keypair_bob = encryption.generate_keypair();

/**

TODO implement this,

ALSO: must pass also for NEW (TODO too) classes implementing:

RSA encr
Ed  sign
NTru encr/sign,   (sign has counter on use count)
Geport sign



	c_pubkey nym_bob = keypair_bob.get_nym();	
	c_plaintext_data plain;
	c_encrypted_data ctext = encryption.encrypt_to( nym_bob::c_pubkey , plain);

	... test all member functions here ...

*/
};


