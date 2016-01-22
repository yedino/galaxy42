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

    c_token process_token_tosend(c_user &, bool fake = 0);

	list<string> inbox;

	bool find_the_cheater (const c_token &, const c_token &);

  public:
	c_user (std::string&);
	c_user (string &&);

	string get_username() const;
	double get_rep();		///< normalize reputation to 0-100 value, approximated by atan()

    c_ed25519 m_edsigner;

    void send_token_bymethod(c_user &, bool fake = 0);

    std::string get_token_packet(c_user &, bool fake = 0);
    void recieve_from_packet(std::string &);

    void send_fake_token(c_user &, size_t = 1);

    void emit_tokens (size_t);
	std::string get_public_key () const;

    bool recieve_token (c_token &token);
};

#endif // COIN_SIGNING_C_USER_HPP
