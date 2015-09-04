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

		c_wallet (const string & = "bla");

		c_wallet (string &&);

		size_t amount ();

		void add_token (const c_token &);

		void remove_token (const c_token &);
};


#endif //COIN_SIGNING_C_WALLET_HPP
