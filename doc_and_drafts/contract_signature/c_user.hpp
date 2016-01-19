#ifndef CONTRACT_SIGNATURE_C_USER_HPP
#define CONTRACT_SIGNATURE_C_USER_HPP
#include <string>
#include "../../crypto_ops/crypto/c_crypto_ed25519.hpp"
#include <list>
#include <stdexcept>
#include <vector>
#include "c_mint.hpp"
#include "c_wallet.hpp"
#include "c_token.hpp"

using std::string;
using std::list;
using std::runtime_error;
using std::vector;

class c_user {
private:
		c_mint m_mint;
		c_wallet m_wallet;
		vector<c_token> used_tokens;

		bool find_the_cheater (const c_token &, const c_token &);

public:
		string nickname;
		c_crypto_ed25519 crypto_api;

		c_user (const string &);

		c_user (string &&);

		list<string> inbox;

		const unsigned char *const get_public_key () const;

		void send_token (c_user &, size_t = 1);

		void send_fake_token (c_user &, size_t = 1);

		void emit_tokens (size_t);

		bool recieve_token (c_token &token);
};


#endif //CONTRACT_SIGNATURE_C_USER_HPP
