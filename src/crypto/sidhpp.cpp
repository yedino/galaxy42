// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "sidhpp.hpp"
#include <SIDH.h>
#include "crypto_basic.hpp"

using namespace antinet_crypto;

std::pair<sodiumpp::locked_string, std::string> sidhpp::generate_keypair()
{
		PCurveIsogenyStaticData curveIsogenyData = &CurveIsogeny_SIDHp751;
		size_t obytes = (curveIsogenyData->owordbits + 7)/8; // Number of bytes in an element in [1, order]
		size_t pbytes = (curveIsogenyData->pwordbits + 7)/8; // Number of bytes in a field element
		const size_t private_key_len = obytes * 2;
		const size_t public_key_len = 4*2*pbytes;
		sodiumpp::locked_string private_key_a(private_key_len);
		sodiumpp::locked_string private_key_b(private_key_len);
		std::fill_n(private_key_a.begin(), private_key_len, 0);
		std::fill_n(private_key_b.begin(), private_key_len, 0);
		std::string public_key_a(public_key_len, 0);
		std::string public_key_b(public_key_len, 0);
		CRYPTO_STATUS status = CRYPTO_SUCCESS;
		PCurveIsogenyStruct curveIsogeny = SIDH_curve_allocate(curveIsogenyData);
		try {
			if (curveIsogeny == nullptr) throw std::runtime_error("SIDH_curve_allocate error");
			status = SIDH_curve_initialize(curveIsogeny, &sidhpp::random_bytes_sidh, curveIsogenyData);
			// generate keys
			status = KeyGeneration_A(
				reinterpret_cast<unsigned char*>(&private_key_a[0]),
				reinterpret_cast<unsigned char *>(&public_key_a[0]),
				curveIsogeny);
			if (status != CRYPTO_SUCCESS) throw std::runtime_error("private key generate error (A)");
			_info("private key a " << to_debug_locked(private_key_a));
			status = KeyGeneration_B(
				reinterpret_cast<unsigned char*>(&private_key_b[0]),
				reinterpret_cast<unsigned char *>(&public_key_b[0]),
				curveIsogeny);
			if (status != CRYPTO_SUCCESS) throw std::runtime_error("private key generate error (B)");
			_info("private key b " << to_debug_locked(private_key_b));

			// check keys valid
			//_info("SIDH validate...");
			bool valid_pub_key = false;
			status = Validate_PKA(
			reinterpret_cast<unsigned char *>(&public_key_a[0]),
				&valid_pub_key,
				curveIsogeny);
			if (status != CRYPTO_SUCCESS) throw std::runtime_error("validate public key error (A)");
			if (!valid_pub_key) throw std::runtime_error("public key (A) is not valid");
			status = Validate_PKB(
			reinterpret_cast<unsigned char *>(&public_key_b[0]),
				&valid_pub_key,
				curveIsogeny);
			if (status != CRYPTO_SUCCESS) throw std::runtime_error("validate public key error (B)");
			if (!valid_pub_key) throw std::runtime_error("public key (B) is not valid");
			assert(public_key_a != public_key_b);
			assert(private_key_a != private_key_b);
		}
		catch(const std::exception &e) {
			SIDH_curve_free(curveIsogeny);
			clear_words(static_cast<void*>(&private_key_a[0]), NBYTES_TO_NWORDS(private_key_len));
			clear_words(static_cast<void*>(&private_key_b[0]), NBYTES_TO_NWORDS(private_key_len));
			clear_words(static_cast<void*>(&public_key_a[0]), NBYTES_TO_NWORDS(public_key_len));
			clear_words(static_cast<void*>(&public_key_b[0]), NBYTES_TO_NWORDS(public_key_len));
			throw e;
		}
		SIDH_curve_free(curveIsogeny);
		sodiumpp::locked_string private_key_main(2 * private_key_len);
		std::copy_n(private_key_a.begin(), private_key_len, private_key_main.begin());
		std::copy_n(private_key_b.begin(), private_key_len, private_key_main.begin() + private_key_len);
		_info("private_key_main " << to_debug_locked(private_key_main));
		std::string public_key_main = public_key_a + public_key_b;
		return std::make_pair(std::move(private_key_main), std::move(public_key_main));
}

sodiumpp::locked_string sidhpp::secret_agreement(const sodiumpp::locked_string &key_self_PRV, const std::string &key_self_pub, const std::string &them_public_key) {
		_dbg1("secret_agreement");
		string them_public_key_a = them_public_key.substr(0, them_public_key.size()/2);
		string them_public_key_b = them_public_key.substr(key_self_pub.size()/2);
		assert( key_self_pub.size() == them_public_key.size());
		assert(them_public_key == them_public_key_a + them_public_key_b);

		sodiumpp::locked_string key_self_PRV_a(key_self_PRV.size() / 2);
		sodiumpp::locked_string key_self_PRV_b(key_self_PRV.size() / 2);

		// this all also assumes that type-A and type-B private keys have size? is this correct? --rafal TODO(rob)
		assert(key_self_PRV.size() == key_self_PRV_a.size() + key_self_PRV_b.size());
		std::copy_n(key_self_PRV.begin(), key_self_PRV.size()/2, key_self_PRV_a.begin());
		auto pos_iterator = key_self_PRV.begin() + (key_self_PRV.size() / 2);
		std::copy_n(pos_iterator, key_self_PRV_b.size(), key_self_PRV_b.begin());
		_info("my private keys");
		_info("private key main " << to_debug_locked(key_self_PRV));
		_info("private key a " << to_debug_locked(key_self_PRV_a));
		_info("private key b " << to_debug_locked(key_self_PRV_b));

		// TODO(rob) make this size-calculations more explained; are they correctd?
		// XXX TODO(rob) there was memory out-of-bounds in demo of SIDH by MS it seems. --rafal
		const size_t shared_secret_size = ((CurveIsogeny_SIDHp751.pwordbits + 7)/8) * 2;
		sodiumpp::locked_string shared_secret_a(shared_secret_size);
		sodiumpp::locked_string shared_secret_b(shared_secret_size);
		std::fill_n(shared_secret_a.begin(), shared_secret_size, 0);
		std::fill_n(shared_secret_b.begin(), shared_secret_size, 0);
		CRYPTO_STATUS status = CRYPTO_SUCCESS;
		// allocate curve
		// TODO move this to class or make global variable
		PCurveIsogenyStaticData curveIsogenyData = &CurveIsogeny_SIDHp751;
		PCurveIsogenyStruct curveIsogeny = SIDH_curve_allocate(curveIsogenyData);
		if (curveIsogeny == nullptr) throw std::runtime_error("SIDH_curve_allocate error");
		status = SIDH_curve_initialize(curveIsogeny, &random_bytes_sidh, curveIsogenyData);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("SIDH_curve_initialize error");

		status = SecretAgreement_A(
			reinterpret_cast<unsigned char *>(&key_self_PRV_a[0]),
			reinterpret_cast<unsigned char *>(&them_public_key_b[0]),
			reinterpret_cast<unsigned char *>(shared_secret_a.buffer_writable()),
			curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("SecretAgreement_A error");

		status = SecretAgreement_B(
			reinterpret_cast<unsigned char *>(&key_self_PRV_b[0]),
			reinterpret_cast<unsigned char *>(&them_public_key_a[0]),
			reinterpret_cast<unsigned char *>(&shared_secret_b[0]),
			curveIsogeny);
		SIDH_curve_free(curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("SecretAgreement_B error");
		using namespace antinet_crypto;
		using namespace string_binary_op;
		sodiumpp::locked_string k_dh_agreed = shared_secret_a ^ shared_secret_b; // the fully agreed key, that is secure result of DH
		return k_dh_agreed;
}

CRYPTO_STATUS sidhpp::random_bytes_sidh(unsigned int nbytes, unsigned char *random_array) {
	static std::ifstream rand_source("/dev/urandom");
	if (nbytes == 0) {
		return CRYPTO_ERROR;
	}
	//static int not_rand = 0;
	for (unsigned int i = 0; i < nbytes; i++) {
		*(random_array + i) = static_cast<unsigned char>(rand_source.get()); // nbytes of random values
		//*(random_array + i) = (unsigned char)not_rand; // XXX
		//++not_rand;
	}
	return CRYPTO_SUCCESS;
}
