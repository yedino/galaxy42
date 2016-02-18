#include "crypto_ed25519.hpp"


namespace crypto_ed25519 {
    keypair generate_key () {
      unsigned char seed[seed_size];
      unsigned char public_key[public_key_size];
      unsigned char private_key[private_key_size];
      ed25519_create_seed(seed);
      ed25519_create_keypair(public_key, private_key, seed);
      return { key(private_key, private_key_size), key(public_key, public_key_size) };
    }

    key sign (const std::string &msg, const keypair &key_) {
      unsigned char signature[signature_size];
      ed25519_sign(signature, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), key_.public_key.m_key.c_str(), key_.private_key.m_key.c_str());
      key sign;
      sign.m_key = ustring(signature, signature_size);
      return sign;
    }

    bool verify_signature (const std::string &msg, const key &signature, const key &public_key) {
      return ed25519_verify(signature.m_key.c_str(), reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), public_key.m_key.c_str()) != 0;
    }

std::ostream &operator<< (std::ostream &os, const key &u) {
  for (auto &c : u.m_key)
    os << std::setfill('0') << std::setw(2) << std::hex << short(c);

  return os;
}

std::istream &operator>> (std::istream &is, key &u) {
  std::string input;
  is >> input;
  for (size_t i = 0; i + 1 < input.size(); i += 2) {
    int r = std::stoi(input.substr(i, 2), nullptr, 16);
    u.m_key += r;
  }
  return is;
}
}
