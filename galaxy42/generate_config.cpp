#include "generate_config.hpp"

void generate_config::crypto_sign(const std::string &filename, e_crypto_strength strength) {

	antinet_crypto::c_multikeys_PAIR keypair;

	switch(strength) {
	case e_crypto_strength::highest:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 1);
	case e_crypto_strength::high:
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
	case e_crypto_strength::normal:
	case e_crypto_strength::fast:
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 1);
	case e_crypto_strength::lowest:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
	}
	keypair.datastore_save_PRV_and_pub(filename);
}

void generate_config::crypto_exchange(const std::string &filename, e_crypto_strength strength) {

	antinet_crypto::c_multikeys_PAIR keypair;

	switch(strength) {
	case e_crypto_strength::highest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_strength::high:
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
	case e_crypto_strength::normal:
	case e_crypto_strength::fast:
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_strength::lowest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
	}
	keypair.datastore_save_PRV_and_pub(filename);
}

void generate_config::any_crypto_set(const std::string &filename,
									 const std::vector<std::pair<antinet_crypto::t_crypto_system_type, int> > &keys) {

	antinet_crypto::c_multikeys_PAIR keypair;

	for (auto &type_and_count : keys) {
		keypair.generate(type_and_count.first, type_and_count.second);
	}
	keypair.datastore_save_PRV_and_pub(filename);
}
