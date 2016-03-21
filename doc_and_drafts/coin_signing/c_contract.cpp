#include "c_contract.hpp"

c_antinet_contract::c_antinet_contract () : m_lifetime(5) {

    //std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    //m_lifetime = std::chrono::duration_cast<std::chrono::seconds>(t_expiration_date.time_since_epoch()).count();


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
