#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP
#include "c_token.hpp"
#include "c_wallet.hpp"
#include <string>
#include "c_mint.hpp"
#include "c_encryption.hpp"

using std::string;

class c_user {
private:
		c_mint mint;
		c_wallet wallet;
		string username;
        string public_key;
        c_ed25519 m_edsigner;
public:
        c_user();
        void send_token (c_user &, size_t);
		void emit_tokens (size_t);
        void recieve_token (string pubkey, c_token, size_t);
};


#endif //COIN_SIGNING_C_USER_HPP
