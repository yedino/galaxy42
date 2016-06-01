#ifndef GENERATE_CONFIG_HPP
#define GENERATE_CONFIG_HPP

#include "filestorage.hpp"
#include "crypto/crypto.hpp"


enum class e_crypto_set : unsigned char {
	lowest = 0,
	fast,
	normal,
	high,
	highest
};


class generate_config {
public:
	generate_config() = delete;

	static void crypto_set (e_crypto_set cryptoset);

};

#endif // GENERATE_CONFIG_HPP
