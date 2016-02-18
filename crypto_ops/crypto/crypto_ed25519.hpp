#ifndef BADZER_CRYPTO_ED25519_HPP
#define BADZER_CRYPTO_ED25519_HPP

#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "ed25519_src/ed25519.h"

using ustring = std::basic_string<unsigned char>;
namespace crypto_ed25519 {
    constexpr size_t public_key_size = 32;
    constexpr size_t private_key_size = 64;
    constexpr size_t signature_size = 64;
    constexpr size_t seed_size = 32;
    constexpr size_t scalar_size = 32;

    typedef ustring private_key_t;
    typedef ustring public_key_t;
    typedef ustring signature_t;

    struct key {
        key () = default;
        key (unsigned char *uc_str, size_t size) : m_key(ustring(uc_str,size))
        { }

        ustring m_key;

        size_t size() const {
            return m_key.size();
        }

        bool operator == (const key &rhs) const {
            return this->m_key == rhs.m_key;
        }
        bool operator != (const key &rhs) const {
            return this->m_key != rhs.m_key;
        }

        template<class Archive>
        void save (Archive & ar, const unsigned int version) const {
            std::basic_string<char> s;
            s.assign (m_key.size (), (char)0);
            for (int i = 0; i < m_key.size (); ++i) {
                s[i] = (char)m_key.at(i);
            }
            ar  & s;
        }

        template<class Archive>
        void load (Archive & ar, const unsigned int version) {
            std::basic_string<char> s;
            ar  & s;
            m_key.assign (s.size (), (unsigned char)0);
            for (int i = 0; i < s.size (); ++i) {
                m_key.at(i) = static_cast<unsigned char>(s.at(i));
            }
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    std::ostream & operator<< (std::ostream &os, const key &u);
    std::istream & operator>> (std::istream &is, key &u);

    struct keypair {
        key private_key;
        key public_key;

        template <typename Archieve>
        void serialize(Archieve &ar, const unsigned version) {
            ar & private_key;
            ar & public_key;
        }
    };

    keypair generate_key ();

    key sign (const std::string &msg, const keypair &key);

    bool verify_signature (const std::string &msg, const key &signature, const key &public_key);
}

using ed_key = crypto_ed25519::key;



#endif //BADZER_CRYPTO_ED25519_HPP
