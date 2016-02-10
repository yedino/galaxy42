#ifndef BADZER_CRYPTO_ED25519_HPP
#define BADZER_CRYPTO_ED25519_HPP

#include <iostream>
#include <string>
#include "ed25519_src/ed25519.h"

using ustring = std::basic_string<unsigned char>;

std::ostream & operator<< (std::ostream &os, const ustring &u);

std::istream & operator>> (std::istream &is, ustring &u);

namespace crypto_ed25519 {
    constexpr size_t public_key_size = 32;
    constexpr size_t private_key_size = 64;
    constexpr size_t signature_size = 64;
    constexpr size_t seed_size = 32;
    constexpr size_t scalar_size = 32;

    typedef ustring private_key_t;
    typedef ustring public_key_t;
    typedef ustring signature_t;

    struct keypair {
        ustring private_key;
        ustring public_key;
    };

    keypair generate_key ();

    ustring sign (const std::string &msg, const keypair &key);

    bool verify_signature (const std::string &msg, const ustring &signature, const ustring &public_key);
}


#endif //BADZER_CRYPTO_ED25519_HPP
