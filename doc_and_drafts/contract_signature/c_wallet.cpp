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

c_wallet::c_wallet (const c_wallet &other) : tokens_type(other.tokens_type), tokens(other.tokens) { }

c_wallet::c_wallet (c_wallet &&other) {
	swap(*this, other);
}


void swap (c_wallet &lhs, c_wallet &rhs) {
	lhs.tokens.swap(rhs.tokens);
	lhs.tokens_type.swap(rhs.tokens_type);
}

c_wallet &c_wallet::operator= (c_wallet wallet) {
	swap(*this, wallet);
	return *this;
}
