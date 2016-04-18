#include "c_contract.hpp"

c_antinet_contract::c_antinet_contract (const std::chrono::seconds &lifetime) : m_lifetime(lifetime.count())
{ }

std::chrono::time_point<std::chrono::system_clock> c_antinet_contract::get_expiration_date () const {
    if(!is_defined || m_sign_date == std::numeric_limits<uint64_t>::max()) {
        throw std::runtime_error("Contract is not defined or have bad sign");
    }
    auto sign_date_timepoint = cs_utils::u64_to_time(m_sign_date);
    return sign_date_timepoint + std::chrono::seconds(m_lifetime);
}


void c_antinet_contract::json_serialize (Json::Value &root) const {
    root["is_defined"] = is_defined;
    root["lifetime"] = static_cast<Json::UInt64>(m_lifetime);
    root["sign"] = std::string(reinterpret_cast<const char *>(m_sign.c_str()),m_sign.size());
    root["sign_date"] = static_cast<Json::UInt64>(m_sign_date);

}
void c_antinet_contract::json_deserialize(Json::Value &root) {
    std::cout << "Contract recieved!" << std::endl; 	// TODO
}


void c_antinet_0001_A::sign_contract(const crypto_ed25519::keypair &keypair) {
    std::string msg  = std::string(reinterpret_cast<const char *>(m_pubkey.c_str()), m_pubkey.size());
    m_sign = crypto_ed25519::sign(msg, keypair);
    m_sign_date = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

}

void c_antinet_0002_A::sign_contract(const crypto_ed25519::keypair &keypair) {
    std::string msg = std::to_string(m_tok_lifetime_hops);
    m_sign = crypto_ed25519::sign(msg, keypair);
    m_sign_date = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

}

void c_antinet_0003_A::sign_contract(const crypto_ed25519::keypair &keypair) {
    std::string msg = std::to_string(m_price) + "|" + std::to_string(amount_tok_min) + "|" + std::to_string(amount_tok_max);
    m_sign = crypto_ed25519::sign(msg, keypair);
    m_sign_date = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

}

void c_antinet_0004_B::sign_contract(const crypto_ed25519::keypair &keypair) {
    std::string msg = std::to_string(m_upload);
    m_sign = crypto_ed25519::sign(msg, keypair);
    m_sign_date = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

}

//void c_contract::json_serialize(Json::Value &root) const {
    // TODO
//}

//void c_contract::json_deserialize(Json::Value &root) {
    // TODO
//}

c_contract::c_contract(const std::string &packet) {
    c_json_serializer::deserialize(*this, packet);
}

c_contract_header c_contract::to_contract_header() {
    return c_contract_header("");
}

std::string c_contract::to_packet(serialization method) {

    std::string packet;
    if(method == serialization::Json) {// json::value
        c_json_serializer::serialize(*this, packet);
    }
    return packet;
}

void c_contract::sign_contract(const crypto_ed25519::keypair &keypair) {
    m_antinet0001.sign_contract(keypair);
    m_antinet0002.sign_contract(keypair);
    m_antinet0003.sign_contract(keypair);
    m_antinet0004.sign_contract(keypair);
}

void c_contract::json_serialize(Json::Value &root) const {
    root["signed antinet contract"] = std::string("aggree");	// TODO
}

void c_contract::json_deserialize(Json::Value &root) {
    std::cout << "Contract recieved!" << std::endl; 	// TODO
}
