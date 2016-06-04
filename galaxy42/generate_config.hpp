#ifndef GENERATE_CONFIG_HPP
#define GENERATE_CONFIG_HPP

#include "libs0.hpp"
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
	static std::string m_crypto_set_name;
};

#endif // GENERATE_CONFIG_HPP
