#ifndef UNTITLED_ECDH_CHACHA20_POLY1305_HPP
#define UNTITLED_ECDH_CHACHA20_POLY1305_HPP
#include <sodium.h>
#include <array>
#include <memory>
#include <cassert>
#include <iomanip>
#include <sstream>

#include <iostream>

namespace ecdh_ChaCha20_Poly1305 {
        typedef std::array<unsigned char, crypto_box_PUBLICKEYBYTES> pubkey_t;
		typedef std::array<unsigned char, crypto_box_SECRETKEYBYTES> privkey_t;
		typedef std::array<unsigned char, crypto_generichash_BYTES> sharedkey_t;
		typedef std::array<unsigned char, crypto_aead_chacha20poly1305_NPUBBYTES> nonce_t;

		struct keypair_t {
				privkey_t privkey;
				pubkey_t pubkey;
		};

        std::string serialize (const unsigned char *data, size_t size);

		template <size_t N>
        std::array<unsigned char, N> deserialize (const std::string &data);

        extern std::array<unsigned char, crypto_box_PUBLICKEYBYTES> deserialize_pubkey(const std::string &data);
        extern std::array<unsigned char, crypto_box_SECRETKEYBYTES> deserialize_privkey(const std::string &data);

        void init ();

        keypair_t generate_keypair ();
        sharedkey_t generate_sharedkey_with (const keypair_t &keypair, const pubkey_t &pubkey);
        nonce_t generate_nonce_with (const keypair_t &keypair, const pubkey_t &pubkey);
        std::string generate_additional_data (const std::string &data);

		std::string encrypt (const std::string &data,
                             const sharedkey_t &sharedkey,
                             const nonce_t &nonce);
        std::string decrypt (const std::string &ciphertext,
                             const sharedkey_t &sharedkey,
                             const nonce_t &nonce);
}


#endif //UNTITLED_ECDH_CHACHA20_POLY1305_HPP
