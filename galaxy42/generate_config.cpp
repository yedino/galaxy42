#include "generate_config.hpp"

void generate_config::crypto_sign(e_crypto_strength strength) {

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
	keypair.datastore_save_PRV_and_pub(generate_config::m_crypto_sign_name);

}

void generate_config::crypto_exchange(e_crypto_strength strength) {

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
	keypair.datastore_save_PRV_and_pub(generate_config::m_crypto_exchange_name);
}

std::string generate_config::m_crypto_sign_name = "";
std::string generate_config::m_crypto_exchange_name = "";

std::string generate_config::get_crypto_exchange_name() {
	return m_crypto_exchange_name;
}

void generate_config::set_crypto_exchange_name(const std::string &crypto_exchange_name) {
	m_crypto_exchange_name = crypto_exchange_name;
}

std::string generate_config::get_crypto_sign_name() {
	return m_crypto_sign_name;
}

void generate_config::set_crypto_sign_name(const std::string &crypto_sign_name) {
	m_crypto_sign_name = crypto_sign_name;
}
