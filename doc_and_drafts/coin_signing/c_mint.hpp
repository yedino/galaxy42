#ifndef C_MINT_H
#define C_MINT_H

#include "libs01.hpp"
#include "coinsign_error.hpp"
#include "c_token.hpp"
#include "c_evidences.hpp"
#include "c_contract.hpp"

class token_id_generator {
  public:
    token_id_generator ();
    size_t generate_id ();

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        UNUSED(version);
        ar & m_id;
    }

  private:
    size_t m_id;
};

class c_mint {
  public:
    c_mint (const std::string &mintname,
            const ed_key &pubkey,
            const std::chrono::seconds &exp_time = std::chrono::hours(72));
    ed_key m_pubkey;
    std::string m_mintname;

    c_token emit_token ();

    bool check_is_emited (const c_token &) const;

    /// get back used token
    /// run find_cheater_token evidence test when double spending is detected
    /// return false if token is OK
    /// throw coin_error expection for cheater finding
    c_contract get_used_token (c_token &&token);

    size_t clean_expired_emited ();
    size_t clean_expired_used ();
    void print_mint_status (std::ostream &os) const;
    size_t get_last_expired_id () const;

  private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        UNUSED(version);
        ar & m_pubkey;
        ar & m_mintname;
        ar & m_emited_tokens;
        ar & m_used_tokens;
        ar & boost::serialization::make_binary_object (&t_expiration_time,sizeof(t_expiration_time));
        ar & m_last_expired_id;
        ar & m_id_generator;
    }

    std::map<c_token, uint64_t> m_emited_tokens;	//< uint64_t for time representation
    std::list<c_token> m_used_tokens;

    /// expitation_time of token
    /// all token emited by this mint should have the same expiration time
    /// This helps in the subsequent token verification
    std::chrono::seconds t_expiration_time;
    size_t m_last_expired_id;

    token_id_generator m_id_generator;
};

#endif // C_MINT_H
