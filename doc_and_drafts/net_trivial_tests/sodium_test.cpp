#include <stdio.h>
#include <sodium.h>

#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <string>


#define MESSAGE (const unsigned char *) "crypto secret message"
#define MESSAGE_LEN 22
#define ADDITIONAL_DATA (const unsigned char *) "123456"
#define ADDITIONAL_DATA_LEN 6


std::string encrypt_decrypt(const std::string &message) {
	std::unique_ptr<unsigned char[]> ciphertext(new unsigned char[message.size() + crypto_aead_chacha20poly1305_ABYTES]);
	unsigned long long ciphertext_len = 0;
	unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES];
	unsigned char key[crypto_aead_chacha20poly1305_KEYBYTES];

	randombytes_buf(nonce, sizeof nonce);
	randombytes_buf(key, sizeof key);
	crypto_aead_chacha20poly1305_encrypt(ciphertext.get(), &ciphertext_len,
										 reinterpret_cast<const unsigned char*>(message.data()), message.size(),
										 ADDITIONAL_DATA, ADDITIONAL_DATA_LEN,
										 nullptr, nonce, key);


	std::unique_ptr<unsigned char[]> decrypted(new unsigned char[message.size()]);
	unsigned long long decrypted_len = 0;
	if (crypto_aead_chacha20poly1305_decrypt(decrypted.get(), &decrypted_len,
											 nullptr,
											 ciphertext.get(), ciphertext_len,
											 ADDITIONAL_DATA,
											 ADDITIONAL_DATA_LEN,
											 nonce, key) != 0) {
		printf("decrypt fail: message forged!\n");
	}
	return std::string(reinterpret_cast<const char*>(decrypted.get()), decrypted_len); // copy
}


int main(int argc, char *argv[]) {

	if(sodium_init() == -1) {
		return 1;
	}
	printf("libsoduim - OK\n");

	const std::string text(2500, 'a');
	const unsigned long long int loops = 100000;
	auto start_point = std::chrono::steady_clock::now();
	for (unsigned long long int i = 0; i < loops; ++i) {
		encrypt_decrypt(text);
	}
	auto stop_point = std::chrono::steady_clock::now();
	auto diff_time = stop_point - start_point;
	std::cout << "time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time).count() << " ms" << std::endl;

	std::cout << "thread test" << std::endl;
	start_point = std::chrono::steady_clock::now();
	for (unsigned long long int i = 0; i < loops; ++i) {
		if (i % 2)
			encrypt_decrypt(text);
		else
			std::async(std::launch::async, encrypt_decrypt, "test");
	}
	stop_point = std::chrono::steady_clock::now();
	diff_time = stop_point - start_point;
	std::cout << "time " << std::chrono::duration_cast<std::chrono::milliseconds>(diff_time).count() << " ms" << std::endl;

	return 0;
}