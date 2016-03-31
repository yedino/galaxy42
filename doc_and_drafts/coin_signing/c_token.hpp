#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP

#include "libs01.hpp"
#include "serialize/c_json_serializer.hpp"

enum class serialization { boost = 1, Json = 2};

struct c_token_header : public ijson_serializable {
    c_token_header () = default;
    c_token_header (const std::string &mintname,
                    const ed_key &mint_pubkey,
                    const size_t id,
                    const uint16_t count,
                    const uint64_t expiration_date);

    void print(std::ostream &os) const;

    std::string m_mintname;
    ed_key m_mint_pubkey;
    size_t m_id;
    uint16_t m_count;
    uint64_t m_expiration_date;

    std::chrono::time_point<std::chrono::system_clock> get_expiration_date () const;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        UNUSED(version);
        ar & m_mintname;
        ar & m_mint_pubkey;
        ar & m_id;
        ar & m_count;
        ar & m_expiration_date;
    }

    virtual ~c_token_header () = default;
};

struct c_contract_header : public ijson_serializable {
    c_contract_header () : m_contract_info("none")
    { }
    c_contract_header (std::string contract_info) : m_contract_info(contract_info)
    { }

    std::string m_contract_info;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        UNUSED(version);
        ar & m_contract_info;
    }

    virtual ~c_contract_header () = default;
};


struct c_chainsign_element : public ijson_serializable {
    c_chainsign_element () = default;
    c_chainsign_element (const std::string &, const ed_key &, const std::string &, const ed_key &);
    c_chainsign_element (const std::string &);	// deserialize chainelement from packet

    std::string m_msg;
    ed_key m_msg_sign;
    std::string m_signer;
    ed_key m_signer_pubkey;

    void print(std::ostream &) const;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

    template <typename Archive>
    void serialize(Archive &ar, const unsigned version) {
        UNUSED(version);
        ar & m_msg;
        ar & m_msg_sign;
        ar & m_signer;
        ar & m_signer_pubkey;
    }
    virtual ~c_chainsign_element () = default;
};

class c_token : public ijson_serializable {
  public:
    c_token () = default;					///< creating empty token


    /// deserialize token
    /// method = 1 : using boost::serialization
    /// method = 2 : using Json::value
    c_token (const std::string &packet, serialization method);
    c_token (const c_token_header &header, const c_contract_header contract = c_contract_header());
    c_token (c_token_header &&header);

    /// serialize token
    /// method = 1 : using boost::serialization
    /// method = 2 : using Json::value
    std::string to_packet (serialization method);

    std::string get_emiter_name () const;
    ed_key get_emiter_pubkey () const;
    size_t get_id () const;
    uint16_t get_count () const;
    c_contract_header get_contract_header() const;
    std::chrono::time_point<std::chrono::system_clock> get_expiration_date () const;

    void increment_count();

    /// verbouse == true : means that for each token all chainsign will be print
    void print (std::ostream &, bool verbouse = 0) const;
    long long get_size () const;
    bool check_date (uint64_t);

    void add_chain_element (const c_chainsign_element &ch);
    void add_chain_element (c_chainsign_element &&ch) noexcept;
    const std::vector<c_chainsign_element>& get_chainsign() const;
    size_t get_chainsign_size () const;

    /// boost::serialization
    friend class boost::serialization::access;

    /// JSONCPP serialize
    virtual void json_serialize (Json::Value &root) const;
    /// JSONCPP deserialize
    virtual void json_deserialize (Json::Value &root);

  private:
    c_token_header m_header;
    c_contract_header m_contract_header;
    std::vector<c_chainsign_element> m_chainsign;

    template<typename Archive>
    void serialize (Archive &ar, const unsigned int version) {
        UNUSED(version);
        ar & m_header;
        ar & m_contract_header;
        ar & m_chainsign;
    }
};

bool operator != (const c_chainsign_element &,const c_chainsign_element &);

bool operator == (const c_token &, const c_token &);

bool operator < (const c_token &, const c_token &);

#endif //COIN_SIGNING_C_TOKEN_HPP
