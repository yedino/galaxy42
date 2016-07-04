// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "../project.hpp"

#if ! ENABLE_CRYPTO_SIDH
	#error "Do not include this header unless you define ENABLE_CRYPTO_SIDH to true. (Maybe you wrongly configured your project, or maybe you are missing such #if condition before your #include)
#endif

#ifndef SIDHPP_HPP
#define SIDHPP_HPP

#include <SIDH_internal.h>
#include "../libs0.hpp"
#include "sodiumpp/locked_string.h"

class sidhpp {
	public:
		/**
		 * @brief generate_keypair
		 * @return std::pair (first = private, second = public)
		 */
		static std::pair<sodiumpp::locked_string, std::string> generate_keypair();
		/**
		 * @brief secret_agreement
		 * @return shared secret as locked string
		 */
		static sodiumpp::locked_string secret_agreement(
			const sodiumpp::locked_string &key_self_PRV,
				  	const std::string &key_self_pub,
					const std::string &them_public_key);
	private:
		static CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array);
};

#endif // SIDHPP_HPP
