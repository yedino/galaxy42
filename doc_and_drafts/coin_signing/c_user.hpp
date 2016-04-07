#ifndef COIN_SIGNING_C_USER_HPP
#define COIN_SIGNING_C_USER_HPP

#include "libs01.hpp"
#include "coinsign_error.hpp"
#include "c_token.hpp"
#include "c_wallet.hpp"
#include "c_locked_queue.hpp"
//#include "c_rpc_bitwallet.hpp"
#include "c_evidences.hpp"
#include "c_mint.hpp"

class c_user {
  public:
    c_user (const std::string& username);
    c_user (std::string && username);
    c_user (c_user && user);

    virtual ~c_user () = default;

    std::string get_username () const;
    void set_username (const string &username);
    ed_key get_public_key () const;
    double get_rep ();		///< normalize reputation to 0-100 value, approximated by atan()

    /// keep_in_wallet = 1 means double spending try
    /// should be used only in tests!
    bool send_token_bymethod (c_user &, bool keep_in_wallet = 0);

    c_token withdraw_token_any (bool keep_in_wallet = 0);
    /// deserialize token
    /// method = 1 : using boost::serialization
    /// method = 2 : using Json::value
    std::string get_token_packet (serialization method, const ed_key &user_pubkey, bool keep_in_wallet = 0);

    bool recieve_from_packet (const string &);
    bool recieve_token (c_token &token);
    void sign_and_push_contract (const ed_key &recipient, c_contract &contract);
    std::vector<c_contract> get_signed_contracts();

    size_t clean_expired_tokens ();
    size_t tokens_refresh ();		///< mostly for clean databases from expiried tokens

    /// Printing tokens status in mint, wallet if verbouse printing all seen tokens and token chainsigns
    void print_status (std::ostream &, bool verbouse = 0) const;

    // mint
    void set_new_mint (const string &mintname,
            const ed_key &pubkey,
            std::chrono::seconds exp_time = std::chrono::hours(72));
    void emit_tokens (size_t);
    long get_mint_last_expired_id () const;


    // saving state
    virtual void save_user (std::string filename = "default") const;	///< "default" filename means m_username.dat file
    virtual bool load_user (std::string filename = "default");		///< "default" filename means m_username.dat file

    void save_coinwallet (const std::string &filename) const;
    void load_coinwallet (const std::string &filename);

    void save_keys () const;
    void load_keys ();

    // bitwallet part
//    bool check_bitwallet ();
//    void set_bitwallet (const std::string &username, const std::string &password, const std::string &address, int port);
//    double get_bitwallet_balance ();

  protected:
    friend class boost::serialization::access;
    template<typename Archive>
    void serialize (Archive &ar, const unsigned int version) {
        UNUSED(version);
        ar & m_edkeys;
        ar & m_mint;
        ar & m_wallet;
        ar & m_seen_tokens;
        ar & m_username;
        ar & m_reputation;
    }
    c_locked_queue<std::pair<ed_key,c_contract>> m_contracts_to_send;
    std::vector<c_contract> m_signed_contracts;

    crypto_ed25519::keypair m_edkeys;

    c_mint m_mint;
    c_wallet m_wallet;
//    c_rpc_bitwallet m_bitwallet;

    std::list<c_token> m_seen_tokens;

    void print_seen_status (std::ostream &) const;

    std::string m_username;
    double m_reputation;

    c_token process_token_tosend (const ed_key &, bool fake = 0);

    std::mutex m_mtx;
};


#endif // COIN_SIGNING_C_USER_HPP
