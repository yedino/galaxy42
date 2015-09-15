#include <iostream>
#include <cstdio>
#include <chrono>
#include "c_market_client.hpp"
#include "c_encryption.hpp"

/////////////////////////////////////////////////// ED 25519 TEST ///////////////////////////////////////////////////

int TEST_c_ed25519(bool verbouse) {

	const unsigned char message[] = "3fd30542fe3f61b14bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
	const int message_len = strlen((char*) message);
	/* create a random seed, and a keypair out of seed */
	c_ed25519 ed25519_obj;


	/* create signature on the message with the keypair */
	unsigned char* pub_key = ed25519_obj.get_public_key_C();
	unsigned char* sign;
	sign = ed25519_obj.sign_C(message, message_len);
	if(verbouse) {
		std::cout << "public_key = ";
		for(int i = 0; i < pub_key_size; ++i) {
			std::cout << std::hex << static_cast<int>(pub_key[i]);
		}
		std::cout << std::endl;

		std::cout << "sing = ";
		for(int i = 0; i < sign_size; ++i) {
		//	std::cout << std::hex << static_cast<int>(sign[i]);
		}
		std::cout << std::endl;

	}
	/* verify the signature */
	if ( ed25519_obj.verify_C(sign, message, message_len, pub_key)) {
		verbouse ? std::printf("valid signature\n"): verbouse = false ;
	} else {
		std::printf("invalid signature\n");
		return 1;
	}
	/* make a slight adjustment and verify again */
	sign[44] ^= 0x10;
	if (ed25519_obj.verify_C(sign, message, message_len, pub_key)) {
		std::printf("did not detect signature change\n");
		return 1;
	} else {
		verbouse ? std::printf("correctly detected signature change\n") : verbouse = false;
	}
	delete [] sign;
	delete [] pub_key;

	return 0;
}
int TEST_cpp_ed25519(bool verbouse) {

	const std::string message = "3fd30542fe3f61b14bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
	const int message_len = message.length();
	/* create a random seed, and a keypair out of seed */
	c_ed25519 ed25519_obj;

	/* create signature on the message with the keypair */
	std::string pub_key = ed25519_obj.get_public_key();
	std::string sign = ed25519_obj.sign(message);
	if(verbouse) {
		std::cout << "public_key = ";
		for(size_t i = 0; i < pub_key.length(); ++i) {
			std::cout << std::hex << static_cast<int>(pub_key.at(i));
		}
		std::cout << std::endl;
		std::cout << "sing = ";
		for(size_t i = 0; i < sign.size(); ++i) {
			std::cout << std::hex << static_cast<int>(sign.at(i));
		}
		std::cout << std::endl;
	}
	/* verify the signature */
	if (ed25519_obj.verify(sign, message, pub_key)) {
		verbouse ? std::printf("valid signature\n"): verbouse = false ;
	} else {
		std::printf("invalid signature\n");
		return 1;
	}

	return 0;
}
void TEST_loop(int test_loop, bool verbouse) {

	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

	for(int i = 0; i < test_loop; ++i) {
		if(TEST_c_ed25519(verbouse)) {
			std::cerr << "signing and validate ed25519 loop test : FAIL" << std::endl;
			return;
		}
	}

	std::chrono::time_point<std::chrono::steady_clock> stop_time = std::chrono::steady_clock::now();
	std::chrono::steady_clock::duration diff = stop_time - start_time;
	std::cerr << "TEST_loop (" << test_loop << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms" << std::endl;

	start_time = std::chrono::steady_clock::now();
	for(int i = 0; i < test_loop; ++i) {
		if(TEST_cpp_ed25519(verbouse)) {
			std::cerr << "signing and validate ed25519 loop test : FAIL" << std::endl;
			return;
		}
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cerr << "TEST_loop (" << test_loop << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for c++ ed25519 members" << std::endl;
}
///
/// Example
///
int main(int argc, char* argv[]) {
	try {
		if (argc != 4) {
			std::cerr << "Usage: blocking_tcp_echo_client <host> <port> <protocol>\n";
			return 1;
		}

		TEST_loop(500,false);

		//c_RSAconnect rsa_link( (std::string(argv[1])), (std::string(argv[2])), (std::string(argv[3])));
		//rsa_link.init();
		c_market_client market_client( (std::string(argv[1])), (std::string(argv[2])), (std::string(argv[3])) );
		market_client.encrypt_client(ed25519);
		market_client.start_market_session();
	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

