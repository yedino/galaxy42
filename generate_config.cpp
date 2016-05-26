#include "generate_config.hpp"


void generate_config::crypto_set(e_crypto_set cryptoset) {

	antinet_crypto::c_multikeys_PAIR keypair;

	switch(cryptoset) {
	case e_crypto_set::highest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_set::high:
	case e_crypto_set::normal:
		keypair.generate(antinet_crypto::e_crypto_system_type_NTRU_EES439EP1, 1);
	case e_crypto_set::fast:
		keypair.generate(antinet_crypto::e_crypto_system_type_SIDH, 1);
	case e_crypto_set::lowest:
		keypair.generate(antinet_crypto::e_crypto_system_type_X25519, 1);
	}
	keypair.datastore_save_PRV_and_pub("current_keys");
}
