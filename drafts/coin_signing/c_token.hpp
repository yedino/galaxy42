#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP
#include <cstdlib>
#include <vector>
#include <iostream>

class token_id_generator {
    static size_t id;
public:
    static size_t generate_id ();
};
struct c_chainsign_element {
    c_chainsign_element(const std::string, const std::string, const std::string);
    const std::string m_signed_msg;
    const std::string m_msg;
    const std::string m_signer_pubkey;
};

struct c_token {
    const size_t id;
    std::vector<c_chainsign_element> m_chainsign;
    c_token (long long);
    bool check_ps(long long);
private:
    long long m_password;
};

bool operator== (const c_token &, const c_token &);

bool operator< (const c_token &, const c_token &);

#endif //COIN_SIGNING_C_TOKEN_HPP

