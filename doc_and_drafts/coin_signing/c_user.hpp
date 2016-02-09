#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP
#include "c_token.hpp"
#include "c_wallet.hpp"
#include "c_rpc_bitwallet.hpp"
#include "c_evidences.hpp"

#include "c_mint.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"
#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <cmath>
#include <mutex>

using std::string;
using std::vector;
using std::list;
using std::runtime_error;

class c_user {
  protected:
    c_ed25519 m_edsigner;
    c_mint m_mint;
	c_wallet m_wallet;
    c_rpc_bitwallet m_bitwallet;
    string m_username;
    vector<c_token> used_tokens;
    void print_used_status(std::ostream &) const;
	double m_reputation;

    c_token process_token_tosend(const std::string &, bool fake = 0);

    c_evidences m_evidences;

    list<string> inbox;

  public:
    c_user () = delete;
    c_user (std::string&);
	c_user (string &&);

	string get_username() const;
    string get_public_key() const;
    double get_rep();		///< normalize reputation to 0-100 value, approximated by atan()

    void send_token_bymethod(c_user &, bool keep_in_wallet = 0); ///< keep_in_wallet = 1 means double spending try
                                                                 /// should be used only in tests!

    std::string get_token_packet(const std::string &user_pubkey, bool keep_in_wallet = 0); ///< keep_in_wallet = 1 means double spending try
                                                                                           /// should be used only in tests!
    void recieve_from_packet(std::string &);

    void double_spend_token(c_user &, size_t = 1);

    void emit_tokens (size_t);
    bool recieve_token (c_token &token);

    void print_status(std::ostream &) const;

    // coin wallet
    void load_coinwallet(const std::string &filename);
    void save_coinwallet(const std::string &filename);

    // bitwallet part
    bool check_bitwallet();
    void set_bitwallet(const std::string &username, const std::string &password, const std::string &address, int port);
    double get_bitwallet_balance();

    std::mutex m_mtx;
};


#endif // COIN_SIGNING_C_USER_HPP
