#include "sidhpp.hpp"
#include <SIDH.h>

std::pair<sodiumpp::locked_string, std::string> sidhpp::generate_keypair()
{
		//_info("SIDH generating...");
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
			status = KeyGeneration_B(
				reinterpret_cast<unsigned char*>(&private_key_b[0]),
				reinterpret_cast<unsigned char *>(&public_key_b[0]),
				curveIsogeny);
			if (status != CRYPTO_SUCCESS) throw std::runtime_error("private key generate error (B)");

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
		std::string public_key_main = public_key_a + public_key_b;
		return std::make_pair(std::move(private_key_main), std::move(public_key_main));
}

CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array)
{
	static std::ifstream rand_source("/dev/urandom");
	if (nbytes == 0) {
		return CRYPTO_ERROR;
	}

	for (unsigned int i = 0; i < nbytes; i++) {
		*(random_array + i) = static_cast<unsigned char>(rand_source.get()); // nbytes of random values
	}
	return CRYPTO_SUCCESS;
}
