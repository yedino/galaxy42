#ifndef COIN_SIGNING_C_WALLET_HPP
#define COIN_SIGNING_C_WALLET_HPP
#include <string>
#include <ctime>
#include <list>
#include "c_token.hpp"


using std::string;
using std::list;

struct c_wallet {
	list<c_token> tokens;
	const string tokens_type;

    c_wallet (const string & = "no named");

	c_wallet (string &&);

	size_t amount ();
    void print_wallet_status(std::ostream &, std::string &my_username);
	void add_token (const c_token &);
    bool process_token() const;
	void remove_token (const c_token &);
};


#endif //COIN_SIGNING_C_WALLET_HPP
