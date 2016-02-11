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

class c_user {
  public:
    c_user () = delete;
    c_user (std::string&);
    c_user (std::string &&);

    std::string get_username () const;
    string get_public_key () const;
    double get_rep ();		///< normalize reputation to 0-100 value, approximated by atan()

    /// keep_in_wallet = 1 means double spending try
    /// should be used only in tests!
    bool send_token_bymethod (c_user &, bool keep_in_wallet = 0);
    std::string get_token_packet (const std::string &user_pubkey, bool keep_in_wallet = 0);

    bool recieve_from_packet (std::string &);
    bool recieve_token (c_token &token);

    size_t clean_expired_tokens();
    size_t tokens_refresh ();		///< mostly for clean databases from expiried tokens
    void print_status (std::ostream &) const;

    // mint
    void set_new_mint (std::string mintname, std::string pubkey, std::chrono::seconds exp_time = std::chrono::hours(72));
    void emit_tokens (size_t);
    long get_mint_last_expired_id () const;

    // coin wallet
    void load_coinwallet (const std::string &filename);
    void save_coinwallet (const std::string &filename);

    // bitwallet part
    bool check_bitwallet ();
    void set_bitwallet (const std::string &username, const std::string &password, const std::string &address, int port);
    double get_bitwallet_balance ();

  protected:
    c_ed25519 m_edsigner;
    c_mint m_mint;
    c_wallet m_wallet;
    c_rpc_bitwallet m_bitwallet;
    std::string m_username;

    std::list<c_token> m_used_tokens;
    void print_used_status (std::ostream &) const;

    double m_reputation;

    c_token process_token_tosend (const std::string &, bool fake = 0);

    c_evidences m_evidences;
    std::mutex m_mtx;
};


#endif // COIN_SIGNING_C_USER_HPP
