#ifndef COIN_SIGNING_C_WALLET_HPP
#define COIN_SIGNING_C_WALLET_HPP
#include <string>
#include "c_token.hpp"
#include <list>

using std::string;
using std::list;

struct c_wallet {
		list<c_token> tokens;
		string tokens_type;

		c_wallet (const string & = "bla");

		c_wallet (string &&);

		c_wallet (const c_wallet &);

		c_wallet (c_wallet &&);

		size_t amount ();

		void add_token (const c_token &);

		void remove_token (const c_token &);

		c_wallet &operator = (c_wallet);
};

void swap (c_wallet &, c_wallet &);


#endif //COIN_SIGNING_C_WALLET_HPP

