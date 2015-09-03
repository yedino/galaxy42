#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP
#include "c_token.hpp"
#include "c_wallet.hpp"
#include <string>
#include "c_mint.hpp"

using std::string;

class c_user {
private:
		c_mint mint;
		c_wallet wallet;
		string username;
		string public_key;
public:
		void send_token (const c_user &, size_t);

		void emit_tokens (size_t);

		void recieve_token (size_t);
};


#endif //COIN_SIGNING_C_USER_HPP
