#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include "c_user.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"

using std::thread;
using std::mutex;
using std::atomic;

int number_of_threads;
atomic<size_t> tests_counter(0);
mutex mtx;

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


bool test_readableEd () {
	std::cout << "RUNNING TEST READABLE_CRYPTO_ED" << std::endl;
	c_ed25519 edtest;
	std::string str_pubkey = edtest.get_public_key();
	unique_ptr<unsigned char[]> utab_pubkey = edtest.get_public_key_uC();
	std::cout << "ed: original utab pubkey = " << utab_pubkey.get() << std::endl;
	std::cout << "ed: string format pubkey = " << str_pubkey << std::endl;
	unique_ptr<unsigned char[]> utab_pubkey_afttrans (readable_toUchar(str_pubkey, pub_key_size));
	std::cout << "ed: back to short format = " << utab_pubkey_afttrans.get() << std::endl;

	const std::string message = "3fd30542fe3f61b24bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
	std::string sign = edtest.sign(message);

	/* verify the signature */
	if (edtest.verify(sign, message, str_pubkey)) {
		std::cout << "valid signature\n" << std::endl;
	} else {
		std::cout << "invalid signature\n" << std::endl;
		return true;
	}

	return false;
}

bool test_manyEdSigning(size_t signs_num, size_t message_len) {
	c_ed25519 edtest;
	std::string str_pubkey = edtest.get_public_key();

	for(size_t i = 0; i < signs_num; ++i) {
		const std::string message = generate_random_string(message_len);
		std::string sign = edtest.sign(message);

		/* verify the signature */
		if (!edtest.verify(sign, message, str_pubkey)) {
			std::cout << "\ninvalid signature!\n" << std::endl;
			return true;
		}
		mtx.lock();
		if( !(i % ((signs_num*number_of_threads)/100))) {
			tests_counter++;

		std::cout << "[" << tests_counter << "%]"
				  << "\b\b\b\b\b\b" << std::flush;
		}
		mtx.unlock();
	}
	return false;
}

bool test_user_sending () {
	std::cout << "RUNNING TEST USER_SENDING" << std::endl;
	c_user test_userA("userA");
	c_user test_userB("userB");

	test_userA.emit_tokens(1);
	test_userA.send_token(test_userB, 1);

	return false;
}

bool test_many_users () {
	std::cout << "RUNNING TEST02 MANY_USER" << std::endl;
	c_user A("userA"), B("userB"), C("userC"), D("userD");

	A.emit_tokens(1);
	A.send_token(B);
	B.send_token(C);
	C.send_token(D);
	D.send_token(A);
	return false;
}

bool test_cheater() {

	std::cout << "RUNNING TEST03 CHEATER" << std::endl;
	c_user A("userA"), B("userB"), C("userC"), X("userX");
	A.emit_tokens(1);
	A.send_token(B);
	B.send_fake_token(C);
	B.send_token(X);
	C.send_token(A);
	X.send_token(A); // should detect cheater
	return false;
}


bool test_all() {

	list<thread> Threads;

	int a = 2000, b = 64;
	std::cout << "RUNNING TEST MANY_ED_SIGNING IN " << number_of_threads << " THREADS" << std::endl;
	for (int i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&a, &b](){test_manyEdSigning(a,b);});
	}

	for (auto &t : Threads) {
		t.join();
	}

	if(     !test_readableEd() &&
			!test_user_sending() &&
			!test_many_users() &&
			!test_cheater() ) {
		return 0;
	} else {
		return 1;
	}
}

int main (int argc, char *argv[]) {
	try {
		ios_base::sync_with_stdio(false);

		if (argc <= 1) {
			cout << "please define number of theards to run test\n";
			return 0;
		}

		number_of_threads = atoi(argv[1]);

		if (number_of_threads <= 0) {
			cout << "please define correct number of theards to run test\n";
			return 1;
		}

		test_all();

		return 0;

	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

