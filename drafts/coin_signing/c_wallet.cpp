#include "c_wallet.hpp"

c_wallet::c_wallet (const string &name) : tokens_type(name) { }

c_wallet::c_wallet (string &&name) : tokens_type(name) { }

size_t c_wallet::amount () {
	return tokens.size();
}

void c_wallet::add_token (const c_token &token) {
	tokens.push_back(token);
}

void c_wallet::remove_token (const c_token &token) {
	tokens.remove(token);
}

