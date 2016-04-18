#ifndef C_CONTRACT_HPP
#define C_CONTRACT_HPP
#include "libs01.hpp"
#include "c_token.hpp"
#include "serialize/c_json_serializer.hpp"

struct c_antinet_contract { //: public ijson_serializable {
    c_antinet_contract (const std::chrono::seconds &lifetime = std::chrono::hours(2));
    virtual std::chrono::time_point<std::chrono::system_clock> get_expiration_date () const;
//    virtual void sign_contract (const crypto_ed25519::keypair &keypair) = 0;

    bool is_defined = false;
    uint64_t m_lifetime = std::numeric_limits<uint64_t>::max();
    uint64_t m_sign_date = std::numeric_limits<uint64_t>::max();
    ed_sign m_sign;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

    virtual ~c_antinet_contract () = default;
};

/**
 * @brief Contract c_antinet_0001_A struct
 * For {lifetime} seconds since signing this contact:
 * I am entity identifiable by my {public_key}
**/
struct c_antinet_0001_A : public c_antinet_contract {
    c_antinet_0001_A () = default;
    c_antinet_0001_A (const std::chrono::seconds &lifetime, const ed_key &pubkey) : c_antinet_contract(lifetime),
                                                                                    m_pubkey(pubkey)
    { }

    virtual void sign_contract (const crypto_ed25519::keypair &keypair);

    ed_key m_pubkey;

    /// JSONCPP serialize
    //virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    //virtual void json_deserialize (Json::Value &root);

    virtual ~c_antinet_0001_A () = default;
};

/**
 * @brief Contract c_antinet_0002_A struct
 * For {lifetime} seconds since signing this contact:
 * I will always agree to offer and then execute contract Contract_antinet_0003_A{with given arguments...}
 * but only under the condition that:
 * the token returned to me has less then {tok_lifetime_hops} hops on it.
 */
struct c_antinet_0002_A  : public c_antinet_contract {
    c_antinet_0002_A () = default;
    c_antinet_0002_A (const std::chrono::seconds &lifetime) : c_antinet_contract(lifetime)
    { }
    virtual void sign_contract (const crypto_ed25519::keypair &keypair);

    uint16_t m_tok_lifetime_hops = 20;

    /// JSONCPP serialize
    //virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    //virtual void json_deserialize (Json::Value &root);

    virtual ~c_antinet_0002_A () = default;
};

/**
 * @brief Contract c_antinet_0003_A struct
 * For {lifetime} seconds since signing this contact:
 * In exchange for defined by you [amount_tok] (between {amount_tok_min} and {amount_tok_max}) of my tokens,
 * I will send you {value}*[amount_tok] of {crypto_currency} in at least {my_reaction_time}
 * seconds after you giving me my tokens.
 */
struct c_antinet_0003_A  : public c_antinet_contract {
    c_antinet_0003_A () = default;
    c_antinet_0003_A (const std::chrono::seconds &lifetime) : c_antinet_contract(lifetime)
    { }

    virtual void sign_contract (const crypto_ed25519::keypair &keypair);

    double m_price = 10;
    size_t amount_tok_min = 1;
    size_t amount_tok_max = 1;

    /// JSONCPP serialize
    //virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    //virtual void json_deserialize (Json::Value &root);

    virtual ~c_antinet_0003_A () = default;
};

/**
 * @brief Contract c_antinet_0004_B struct
 * Contract_antinet_0004_B is simplified version of _A:
 * For {lifetime} seconds since signing this contact:
 * In exchange for defined by you:
 * [amount_tok1] (between {amount_tok1_min} and {amount_tok1_max}) of tokens by {token_mint}
 *
 * I will grant you increased peering speed in {network_type},
 * - increase up/down speed will be set to {network_speed}
 *
 * for following duration (exceeding any of then ends the grant):
 * - for {network_data_updown}*[amount_tok1] bytes received or sent
 */
struct c_antinet_0004_B  : public c_antinet_contract {
    c_antinet_0004_B () = default;
    virtual void sign_contract (const crypto_ed25519::keypair &keypair);
    c_antinet_0004_B (const std::chrono::seconds &lifetime) : c_antinet_contract(lifetime)
    { }

    uint64_t m_upload = 10;

    /// JSONCPP serialize
    //virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    //virtual void json_deserialize (Json::Value &root);

    virtual ~c_antinet_0004_B () = default;
};

class c_contract : public ijson_serializable {
public:

    c_contract () = default;
    c_contract(const std::string &packet);

    c_contract(const c_antinet_0001_A &a1,
               const c_antinet_0002_A &a2,
               const c_antinet_0003_A &a3,
               const c_antinet_0004_B &a4) : m_antinet0001(a1),
                                             m_antinet0002(a2),
                                             m_antinet0003(a3),
                                             m_antinet0004(a4)
    { }

    c_contract_header to_contract_header();
    std::string to_packet(serialization method = serialization::Json);

    void sign_contract (const crypto_ed25519::keypair &keypair);

    c_antinet_0001_A m_antinet0001;
    c_antinet_0002_A m_antinet0002;
    c_antinet_0003_A m_antinet0003;
    c_antinet_0004_B m_antinet0004;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

    virtual ~c_contract() = default;
};

#endif // C_CONTRACT_HPP
