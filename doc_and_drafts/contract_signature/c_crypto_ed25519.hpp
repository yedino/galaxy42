/* See the LICENSE.txt for license information */

#ifndef CRYPTO_C_ED25519_H
#define CRYPTO_C_ED25519_H
#include "ed25519_src/ed25519.h"
#include "ed25519_src/ge.h"
#include "ed25519_src/sc.h"
#include <string>
#include <iostream>
#include <stdexcept>

using std::string;
using std::cin;
using std::cout;

class c_crypto_ed25519 {
private:
		unsigned char public_key[32], private_key[64], seed[32], scalar[32];

public:
		c_crypto_ed25519 () = default;

		void generate_key ();

		void sign (const string &, unsigned char *) const; // signature is unsigned char [64]

		bool verify_sign (const string &, const unsigned char *, const unsigned char *) const;

		const unsigned char *const get_public_key () const;
};

#endif //CRYPTO_C_ED25519_H
