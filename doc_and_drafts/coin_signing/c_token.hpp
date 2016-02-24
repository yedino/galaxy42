#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP
#include <cstdlib>
#include <vector>
#include <iostream>
#include <limits>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include "../../crypto_ops/crypto/crypto_ed25519.hpp"

#include <boost/serialization/binary_object.hpp>

struct c_token_header {
    c_token_header () = default;
    c_token_header (const std::string &mintname,
                    const ed_key &mint_pubkey,
                    const size_t id,
                    const long long password,
                    const std::chrono::time_point<std::chrono::system_clock> expiration_date);

    void print(std::ostream &os) const;

    size_t m_id;
    std::string m_mintname;
    ed_key m_mint_pubkey;
    long long m_password;
    std::chrono::time_point<std::chrono::system_clock> m_expiration_date;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        ar & m_mintname;
        ar & m_mint_pubkey;
        ar & m_id;
        ar & m_password;
        ar & boost::serialization::make_binary_object (&m_expiration_date,sizeof(m_expiration_date));
    }
};

struct c_chainsign_element {
    c_chainsign_element () = default;
    c_chainsign_element (const std::string, const ed_key, const std::string, const ed_key);
    c_chainsign_element (const std::string &);	// deserialize chainelement from packet

    std::string m_msg;
    ed_key m_msg_sign;
    std::string m_signer;
    ed_key m_signer_pubkey;

    void print(std::ostream &) const;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        ar & m_msg;
        ar & m_msg_sign;
        ar & m_signer;
        ar & m_signer_pubkey,m_signer_pubkey;
    }
};

class c_token {
  public:
    c_token () = default;
    c_token (std::string);		///< deserialize token from recived packet
    c_token (const c_token_header &header);
    c_token (c_token_header &&header);

    std::string to_packet ();	///< serialize token

    std::string get_emiter_name () const;
    ed_key get_emiter_pubkey () const;
    size_t get_id () const;
    std::chrono::time_point<std::chrono::system_clock>  get_expiration_date () const;

    /// verbouse == true : means that for each token all chainsign will be print
    void print (std::ostream &, bool verbouse = 0) const;
    long long get_size () const;
    bool check_ps (long long);

    void add_chain_element (const c_chainsign_element &ch);
    void add_chain_element (c_chainsign_element &&ch) noexcept;
    const std::vector<c_chainsign_element>& get_chainsign() const;
    const size_t get_chainsign_size () const;

    friend class boost::serialization::access;
  private:
    c_token_header m_header;
    std::vector<c_chainsign_element> m_chainsign;

    template<typename Archive>
    void serialize (Archive &ar, const unsigned int version)
    {
        ar & m_header;
        ar & m_chainsign;
    }
};

bool operator != (const c_chainsign_element &,const c_chainsign_element &);

bool operator == (const c_token &, const c_token &);

bool operator < (const c_token &, const c_token &);

#endif //COIN_SIGNING_C_TOKEN_HPP
