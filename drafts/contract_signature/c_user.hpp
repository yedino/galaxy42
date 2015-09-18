#ifndef CONTRACT_SIGNATURE_C_USER_HPP
#define CONTRACT_SIGNATURE_C_USER_HPP
#include <string>
#include "c_crypto_ed25519.hpp"
#include <list>
#include <stdexcept>

using std::string;
using std::list;
using std::runtime_error;

class c_user {
public:
		string nickname;
		c_crypto_ed25519 crypto_api;

		c_user (const string &);

		c_user (string &&);

		list<string> inbox;

		const unsigned char *const get_public_key () const;
};


#endif //CONTRACT_SIGNATURE_C_USER_HPP
