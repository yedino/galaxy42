#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP
#include "c_token.hpp"
#include "c_wallet.hpp"
#include <string>
#include "c_mint.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"
#include <vector>
#include <list>
#include <stdexcept>
#include <cmath>

using std::string;
using std::vector;
using std::list;
using std::runtime_error;

class c_user {
  private:
	c_mint m_mint;
	c_wallet m_wallet;
	string m_username;
	string m_public_key;
	vector<c_token> used_tokens;
	double m_reputation;

	list<string> inbox;

	bool find_the_cheater (const c_token &, const c_token &);

  public:
	c_user (std::string&);
	c_user (string &&);

	string get_username() const;
	double get_rep();		/// normalize reputation to 0-100 value, approximated by atan()

	c_ed25519 m_edsigner;
	void send_token (c_user &, size_t = 1);
	void send_fake_token(c_user &, size_t = 1);
	void emit_tokens (size_t);
	std::string get_public_key () const;

	bool recieve_token (c_token &token, size_t = 1);
};

#endif // COIN_SIGNING_C_USER_HPP
