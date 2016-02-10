#include "crypto_ed25519.hpp"

std::ostream &operator<< (std::ostream &os, const ustring &u) {
  for (auto &c : u)
    os << setfill('0') << setw(2) << std::hex << short(c);

  return os;
}

std::istream &operator>> (std::istream &is, ustring &u) {
  std::string input;
  is >> input;
  for (size_t i = 0; i + 1 < input.size(); i += 2) {
    int r = std::stoi(input.substr(i, 2), nullptr, 16);
    u += r;
  }
  return is;
}

namespace crypto_ed25519 {
    keypair generate_key () {
      unsigned char seed[seed_size];
      unsigned char public_key[public_key_size];
      unsigned char private_key[private_key_size];
      ed25519_create_seed(seed);
      ed25519_create_keypair(public_key, private_key, seed);
      return { ustring(private_key, private_key_size), ustring(public_key, public_key_size) };
    }

    ustring sign (const std::string &msg, const keypair &key) {
      unsigned char signature[signature_size];
      ed25519_sign(signature, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), key.public_key.c_str(), key.private_key.c_str());
      return ustring(signature, signature_size);
    }

    bool verify_signature (const std::string &msg, const ustring &signature, const ustring &public_key) {
      return ed25519_verify(signature.c_str(), reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), public_key.c_str()) != 0;
    }
}
