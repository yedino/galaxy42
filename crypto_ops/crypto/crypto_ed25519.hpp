#ifndef BADZER_CRYPTO_ED25519_HPP
#define BADZER_CRYPTO_ED25519_HPP

#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "ed25519_src/ed25519.h"

namespace crypto_ed25519 {
		constexpr size_t public_key_size = 32;
		constexpr size_t private_key_size = 64;
		constexpr size_t signature_size = 64;
		constexpr size_t seed_size = 32;
		constexpr size_t scalar_size = 32;


		struct ustring {
				ustring () = default;

				ustring (const unsigned char *uc_str, size_t size) : m_string(uc_str, size) { }
				ustring (const ustring &uc_str) : m_string(uc_str.m_string) { }
				ustring (ustring &&uc_str) : m_string(std::move(uc_str.m_string)) { }

				std::basic_string<unsigned char> m_string;

				size_t size () const {
					return m_string.size();
				}
				const unsigned char *c_str () const noexcept {  ///< Be carefull, unsigned* could contain NULL after converting to char*
					return m_string.c_str();
				}

				ustring &operator= (const ustring &rhs) = default;

				bool operator== (const ustring &rhs) const {
					return this->m_string == rhs.m_string;
				}

				bool operator!= (const ustring &rhs) const {
					return this->m_string != rhs.m_string;
				}

				template <class Archive>
				void save (Archive &ar, const unsigned int version) const {
					if (version != 0) {
						std::cout << "boost::archieve ustring save version:" << version << std::endl;
					}
					std::basic_string<char> s;
					s.assign(m_string.size(), (char)0);
					for (size_t i = 0; i < m_string.size(); ++i) {
						s[i] = (char)m_string.at(i);
					}
					ar & s;
				}
				template <class Archive>
				void load (Archive &ar, const unsigned int version) {
					if (version != 0) {
						std::cout << "boost::archieve ustring load version:" << version << std::endl;
					}
					std::basic_string<char> s;
					ar & s;
					m_string.assign(s.size(), static_cast<unsigned char>(0));
					for (size_t i = 0; i < s.size(); ++i) {
						m_string.at(i) = static_cast<unsigned char>(s.at(i));
					}
				}

				BOOST_SERIALIZATION_SPLIT_MEMBER()
		};

		typedef ustring private_key_t;
		typedef ustring public_key_t;
		typedef ustring signature_t;

		std::ostream &operator<< (std::ostream &os, const ustring &u);

		std::istream &operator>> (std::istream &is, ustring &u);

		struct keypair {
				ustring private_key;
				ustring public_key;

				template <typename Archieve>
				void serialize (Archieve &ar, const unsigned version) {
					if (version != 0) {
						std::cout << "boost::archieve keypair serialize version:" << version << std::endl;
					}
					ar & private_key;
					ar & public_key;
				}
		};

		keypair generate_key ();

		ustring sign (const std::string &msg, const keypair &keys);

		bool verify_signature (const std::string &msg, const ustring &signature, const ustring &public_key);
}

using ed_key = crypto_ed25519::ustring;
using ed_sign = crypto_ed25519::ustring;

#endif //BADZER_CRYPTO_ED25519_HPP
