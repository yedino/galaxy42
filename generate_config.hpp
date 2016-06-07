#ifndef GENERATE_CONFIG_HPP
#define GENERATE_CONFIG_HPP

#include "libs0.hpp"
#include "filestorage.hpp"
#include "crypto/crypto.hpp"


enum class e_crypto_strength : unsigned char {
	lowest = 0,
	fast,
	normal,
	high,
	highest,
};


class generate_config {
public:
	generate_config() = delete;

	/**
	 * @brief crypto_permanent Create set of permanent keys with ability to sign
	 * @param strength
	 */
	static void crypto_sign (e_crypto_strength strength);
	/**
	 * @brief crypto_current Create set of current keys with ability to make DH exchange
	 * @param strength
	 */
	static void crypto_exchange (e_crypto_strength strength);

	static void any_crypto_set(const std::string &filename,
							   const std::vector<std::pair<antinet_crypto::t_crypto_system_type,int>> &keys);

	static std::string get_crypto_sign_name();
	static std::string get_crypto_exchange_name();

	static void set_crypto_sign_name(const std::string &crypto_sign_name);
	static void set_crypto_exchange_name(const std::string &crypto_exchange_name);

private:
	static std::string m_crypto_sign_name;
	static std::string m_crypto_exchange_name;

};

#endif // GENERATE_CONFIG_HPP
