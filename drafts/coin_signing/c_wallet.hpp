#ifndef COIN_SIGNING_C_WALLET_HPP
#define COIN_SIGNING_C_WALLET_HPP
#include <string>
#include "c_token.hpp"
#include <list>

using std::string;
using std::list;

struct c_wallet {
		list<c_token> tokens;
		const string tokens_type;

		c_wallet (const string &name);

		c_wallet (string &&name);

		size_t amount ();
};


#endif //COIN_SIGNING_C_WALLET_HPP
