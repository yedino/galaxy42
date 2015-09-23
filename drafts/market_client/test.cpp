#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <list>
#include "c_market_client.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"
#include "../../crypto_ops/crypto/c_crypto_ed25519.hpp"
string generate_random_string (size_t length);

using std::thread;
using std::mutex;

size_t number_of_threads;
std::atomic<size_t> tests_counter(0);
mutex mtx;


/////////////////////////////////////////////////// ED 25519 TEST ///////////////////////////////////////////////////

int TEST_c_ed25519(bool verbouse) {

	const unsigned char message[] = "3fd30542fe3f61b14bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
	const int message_len = strlen((char*) message);
	/* create a random seed, and a keypair out of seed */
	c_ed25519 ed25519_obj;

	/* create signature on the message with the keypair */
	unsigned char pub_key[pub_key_size];
	unsigned char sign[sign_size];
	ed25519_obj.get_public_key_C(pub_key);
	ed25519_obj.sign_C(message, message_len, sign);
	if (verbouse) {
		std::cout << "public_key = ";
		for(int i = 0; i < pub_key_size; ++i) {
			std::cout << std::hex << static_cast<int>(pub_key[i]);
		}
		std::cout << "\nsing = ";
		for(int i = 0; i < sign_size; ++i) {
			std::cout << std::hex << static_cast<int>(sign[i]);
		}
		std::cout << std::endl;

	}
	/* verify the signature */
	if (ed25519_obj.verify_C(sign, message, message_len, pub_key)) {
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

	return 0;
}
int TEST_cpp_ed25519(bool verbouse) {
	const std::string message = generate_random_string(64);
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
		std::cout << std::dec << std::endl;
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

int TEST_speedtest01_ed25519(size_t loop_num) {
	c_ed25519 ed25519_obj;
	std::string pub_key = ed25519_obj.get_public_key();

	tests_counter = 0;
	for(size_t i = 0; i < loop_num; i++) {
		const std::string message = generate_random_string(64);
		std::string sign = ed25519_obj.sign(message);
		if (ed25519_obj.verify(sign, message, pub_key)) {
		} else {
			std::printf("invalid signature\n");
			return 1;
		}
		mtx.lock();
		if( !(i % ((loop_num*number_of_threads)/100))) {
			tests_counter++;

		std::cout << "[" << tests_counter << "%]"
				  << "\b\b\b\b\b\b" << std::flush;
		}
		mtx.unlock();
	}
	return 0;
}

int TEST_speedtest02_ed25519 (size_t loop_num) {
	c_crypto_ed25519 ed25519_obj;
	ed25519_obj.generate_key();
	unsigned char Signature [64];

	tests_counter = 0;
	for (size_t i = 0; i < loop_num; i++) {
		const std::string message = generate_random_string(64);
		ed25519_obj.sign(message, Signature);
		if (ed25519_obj.verify_sign(message, Signature, ed25519_obj.get_public_key())) {
		} else {
			std::printf("invalid signature\n");
			return 1;
		}
		mtx.lock();
		if( !(i % ((loop_num*number_of_threads)/100))) {
			tests_counter++;

		std::cout << "[" << tests_counter << "%]"
				  << "\b\b\b\b\b\b" << std::flush;
		}
		mtx.unlock();
	}
	return 0;
}
int TEST_speedtest03c_ed25519(size_t loop_num) {
	c_ed25519 ed25519_obj;
	unsigned char pub_key[pub_key_size];
	unsigned char sign[sign_size];
	std::string message;
	ed25519_obj.get_public_key_C(pub_key);

	tests_counter = 0;
	for (size_t i = 0; i < loop_num; i++) {
		message = generate_random_string(64);
		ed25519_obj.sign_C(reinterpret_cast<const unsigned char *>(message.c_str()), 64, sign);
		if (ed25519_obj.verify_C(sign, reinterpret_cast<const unsigned char *>(message.c_str()), 64, pub_key)) {
		} else {
			std::printf("invalid signature\n");
		return 1;
		}
		mtx.lock();
		if( !(i % ((loop_num*number_of_threads)/100))) {
			tests_counter++;

		std::cout << "[" << tests_counter << "%]"
				  << "\b\b\b\b\b\b" << std::flush;
		}
		mtx.unlock();
	}
	return 0;
}
/////////////////////////////////////////////////// RSA TEST ///////////////////////////////////////////////////

string generate_random_string (size_t length) {
	auto generate_random_char = [] () -> char {
		const char Charset[] = "0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(Charset) - 1);
		return Charset[rand() % max_index];
	};
	string str(length, 0);
	generate_n(str.begin(), length, generate_random_char);
	return str;
}

bool TEST_rsaSign(size_t loop_num) {
	c_RSA testRSA;
	std::string pub_key = testRSA.get_public_key();
	std::string msg;
	std::string sign;

	tests_counter = 0;
	for(size_t i = 0; i < loop_num; ++i) {
		msg = generate_random_string(64);
		sign = testRSA.sign(msg);

		if(! testRSA.verify(sign,msg,pub_key)) {
			std::cerr << "error: RSA sign verify fail!" << std::endl;
			return true;
		}
		mtx.lock();
		if( !(i % ((loop_num*number_of_threads)/100))) {
			tests_counter++;

		std::cout << "[" << tests_counter << "%]"
				  << "\b\b\b\b\b\b" << std::flush;
		}
		mtx.unlock();
	}
	return false;
}

void TEST_all(int loop_num, bool verbouse) {

	std::chrono::time_point<std::chrono::steady_clock> start_time, stop_time;
	std::chrono::steady_clock::duration diff;
	std::list<thread> Threads;

	start_time = std::chrono::steady_clock::now();
	for(int i = 0; i < loop_num; ++i) {
		if(TEST_c_ed25519(verbouse)) {
			std::cerr << "signing and validate ed25519 loop test : FAIL" << std::endl;
			return;
		}
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms" << std::endl;

	start_time = std::chrono::steady_clock::now();
	for(int i = 0; i < loop_num; ++i) {
		if(TEST_cpp_ed25519(verbouse)) {
			std::cerr << "signing and validate ed25519 loop test : FAIL" << std::endl;
			return;
		}
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for c++ ed25519 members" << std::endl;

	start_time = std::chrono::steady_clock::now();
	for (size_t i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&loop_num](){TEST_speedtest02_ed25519(loop_num/number_of_threads);});
	}
	for (auto &t : Threads) {
		t.join();
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for kuba shit lib, only signing" << std::endl;
	Threads.clear();

	start_time = std::chrono::steady_clock::now();
	for (size_t i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&loop_num](){TEST_speedtest01_ed25519(loop_num/number_of_threads);});
	}
	for (auto &t : Threads) {
		t.join();
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for damian master lib, only signing" << std::endl;
	Threads.clear();

	start_time = std::chrono::steady_clock::now();
	for (size_t i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&loop_num](){TEST_speedtest03c_ed25519(loop_num/number_of_threads);});
	}
	for (auto &t : Threads) {
		t.join();
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for damian C lib, only signing" << std::endl;
	Threads.clear();

	start_time = std::chrono::steady_clock::now();
	for (size_t i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&loop_num](){TEST_rsaSign(loop_num/number_of_threads);});
	}
	for (auto &t : Threads) {
		t.join();
	}
	stop_time = std::chrono::steady_clock::now();
	diff = stop_time - start_time;
	std::cout << "TEST_loop (" << loop_num << ") PASSED in " << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			  << " ms for RSA " << key_size << " signing and validate test" << std::endl;
	Threads.clear();
}
///
/// Example
///
int main(int argc, char* argv[]) {
	try {
		if (argc != 4) {
			std::cerr << "Usage: market_client <host> <port> <protocol>\n";
			return 1;
		}
		ios_base::sync_with_stdio(false);
		number_of_threads = 2;
		std::cout << "RUNNING TESTS WITH " << number_of_threads << " THREADS" << std::endl;

		size_t test_loop = 100;
		bool verbouse = false;
		TEST_all(test_loop,verbouse);

		//c_market_client market_client( (std::string(argv[1])), (std::string(argv[2])), (std::string(argv[3])) );
		c_UDPasync_client market_client( (std::string(argv[1])), (std::string(argv[2])), (atoi(argv[3])) );
		market_client.encrypt_client(ed25519);
		//market_client.start_market_session();
		market_client.start_async_session();

	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

