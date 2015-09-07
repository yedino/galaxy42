#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP
#include "c_token.hpp"
#include "c_wallet.hpp"
#include <string>
#include "c_mint.hpp"
#include "c_encryption.hpp"
#include <vector>

using std::string;
using std::vector;

class c_user {
private:
		c_mint m_mint;
		c_wallet m_wallet;
		string m_username;
		string m_public_key;
		c_ed25519 m_edsigner;
		vector<c_token> used_tokens;

        bool find_the_cheater (const c_token &, const c_token &);

public:
		c_user (std::string);

        string get_username();
		void send_token (c_user &, size_t = 1);

		void emit_tokens (size_t);

		bool recieve_token (c_token &token, size_t = 1);
};


#endif //COIN_SIGNING_C_USER_HPP
