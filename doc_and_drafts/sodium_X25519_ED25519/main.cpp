//g++ -o main main.cpp -lsodium

#include <iostream>
#include <cstring>
#include <sodium.h>

void ed25519_for_key_exchange() {

        unsigned char Alice_pk[crypto_sign_PUBLICKEYBYTES];
        unsigned char Alice_sk[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair(Alice_pk, Alice_sk); // generate keys for signing

        unsigned char Bob_pk[crypto_sign_PUBLICKEYBYTES];
        unsigned char Bob_sk[crypto_sign_SECRETKEYBYTES];
        crypto_sign_keypair(Bob_pk, Bob_sk);


        unsigned char Alice_shared[crypto_scalarmult_BYTES];
        crypto_scalarmult(Alice_shared, Alice_sk, Bob_pk);

        unsigned char Bob_shared[crypto_scalarmult_BYTES];
        crypto_scalarmult(Bob_shared, Bob_sk, Alice_pk);

        std::cout << "shared cmp " << sodium_memcmp(Alice_shared, Bob_shared, crypto_scalarmult_BYTES) << std::endl;
        std::cout << "std cmp " << memcmp(Alice_shared, Bob_shared, crypto_scalarmult_BYTES) << std::endl;

}

void x25519_for_sign() {
        unsigned char Alice_pk[crypto_box_SECRETKEYBYTES];
        unsigned char Alice_sk[crypto_box_PUBLICKEYBYTES];
	randombytes_buf(Alice_sk, crypto_box_SECRETKEYBYTES);
	crypto_scalarmult_base(Alice_pk, Alice_sk); // generate keys for exchange
	
	#define MESSAGE (const unsigned char *) "test"
	#define MESSAGE_LEN 4
	unsigned char signed_message[crypto_sign_BYTES + MESSAGE_LEN];
	unsigned long long signed_message_len;
	crypto_sign(signed_message, &signed_message_len, MESSAGE, MESSAGE_LEN, Alice_sk);

	unsigned char unsigned_message[MESSAGE_LEN];
	unsigned long long unsigned_message_len;

	if (crypto_sign_open(unsigned_message, &unsigned_message_len,
                     signed_message, signed_message_len, Alice_pk) != 0) {
		std::cout << "Incorrect signature!" << std::endl;
	}
	else {
		std::cout << "signature OK" << std::endl;
	}
}

int main () {
	if (sodium_init() == -1) {
		std::cout << "init 0" << std::endl;
		return 1;
	}

	ed25519_for_key_exchange();
	x25519_for_sign();
	return 0;
}
