#ifndef C_MINT_H
#define C_MINT_H

#include <chrono>
#include "c_token.hpp"
#include "c_evidences.hpp"
#include "coinsign_error.hpp"
#include "../../crypto_ops/crypto/c_random_generator.hpp"
#include <map>
#include <list>
#include <string>

class token_id_generator {
  public:
    token_id_generator ();
    size_t generate_id ();
  private:
    size_t id;
};

class c_mint {
  public:
    c_mint (const std::string &mintname,
            const ed_key &pubkey,
            const std::chrono::seconds &exp_time = std::chrono::hours(72));
    std::string m_mintname;
    ed_key m_pubkey;

    c_token emit_token ();

    bool check_is_emited (const c_token &) const;

    /// get back used token
    /// run find_cheater_token evidence test when double spending is detected
    /// return false if token is OK
    /// throw coin_error expection for cheater finding
    bool get_used_token (c_token &);

    size_t clean_expired_emited ();
    size_t clean_expired_used ();
    void print_mint_status (std::ostream &os) const;
    size_t get_last_expired_id () const;
  private:
	std::map<c_token, long long> m_emited_tokens;
    std::list<c_token> m_used_tokens;
    c_random_generator<long long> random_generator;

    size_t m_last_expired_id;

    /// expitation_time of token
    /// all token emited by this mint should have the same expiration time
    /// This helps in the subsequent token verification
    std::chrono::seconds t_expiration_time;

    long long generate_password ();
    token_id_generator m_id_generator;
};

#endif // C_MINT_H
