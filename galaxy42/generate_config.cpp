#include "generate_config.hpp"

void generate_config::crypto_permanent(e_crypto_strength strength) {

	antinet_crypto::c_multikeys_PAIR keypair;

	switch(strength) {
	case e_crypto_strength::highest:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 2);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 2);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 2);
	case e_crypto_strength::high:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 1);
	case e_crypto_strength::normal:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 1);
	case e_crypto_strength::fast:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_sign, 1);
	case e_crypto_strength::lowest:
		keypair.generate(antinet_crypto::e_crypto_system_type_Ed25519, 1);
	}
	keypair.datastore_save_PRV_and_pub(generate_config::m_crypto_permanent_name);

}

void generate_config::crypto_current(e_crypto_strength strength) {

	antinet_crypto::c_multikeys_PAIR keypair;

	switch(strength) {
	case e_crypto_strength::highest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 2);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 2);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 2);
	case e_crypto_strength::high:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_strength::normal:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_strength::fast:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_strength::lowest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
	}
	keypair.datastore_save_PRV_and_pub(generate_config::m_crypto_current_name);
}

std::string generate_config::m_crypto_permanent_name = "";
std::string generate_config::m_crypto_current_name = "";

std::string generate_config::get_crypto_current_name()
{
	return m_crypto_current_name;
}

void generate_config::set_crypto_current_name(const std::string &crypto_current_name)
{
	m_crypto_current_name = crypto_current_name;
}

std::string generate_config::get_crypto_permanent_name() {
	return m_crypto_permanent_name;
}

void generate_config::set_crypto_permanent_name(const std::string &crypto_permanent_name) {
	m_crypto_permanent_name = crypto_permanent_name;
}
