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
        c_mint m_mint;
        c_wallet m_wallet;
        string m_username;
        string m_public_key;
        c_ed25519 m_edsigner;
public:
        c_user(std::string username = "test");
        void send_token (c_user &, size_t amount = 1);
		void emit_tokens (size_t);
        void recieve_token (c_token &token, size_t amount = 1);
};


#endif //COIN_SIGNING_C_USER_HPP
