#include <iostream>
#include <string>
#include "c_crypto_geport.hpp"
#include "c_random_generator.hpp"
#include "c_crypto_ed25519.hpp"
#include <list>

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::list;

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

string generate_different_string (const string &msg) {
	if (rand() % 3 == 0)
		return msg + ' ';

	string ret = msg;

	if (rand() % 3 == 1) {
		ret.erase(0, 1);
	}

	if (ret[ret.size() / 2] != 'a')
		ret[ret.size() / 2] = 'a';
	else
		ret[ret.size() / 2] = '4';
	return ret;
}

signed_msg generate_different_signature (const signed_msg &signature) {
	signed_msg ret = signature;
	if (rand() % 2)
		ret.public_key += (rand() % 10000000L) + 1;
	else
		ret.Signature[rand() % (256 + 8)] -= (rand() % 10000000L) + 1;

	return ret;
}

void random_generator_test (size_t size) {
	c_random_generator<c_crypto_geport::long_type> generator;
	list <c_crypto_geport::long_type> set;

	for (size_t i = 0; i < size; ++i)
		set.push_back(generator.get_random(33));

	size_t counter = set.size();
	set.unique();
	cout << "there was " << counter - set.size() << " colisions" << endl;
}

void correctness_test (size_t size) {
	size_t jump = 100;
	c_crypto_geport geport;
	c_crypto_geport::long_type Private_key[256 + 8];
	geport.generate_private_key(Private_key);

	string message, different_message;
	signed_msg signature, different_signature;

	for (size_t i = 0; i < size; ++i) {
		message = generate_random_string((size_t)(rand() % 50) + 5);
		signature = geport.sign(message, Private_key);
		different_signature = generate_different_signature(signature);
		different_message = generate_different_string(message);
		if (!geport.verify_sign(message, signature) || geport.verify_sign(different_message, signature) ||
				geport.verify_sign(message, different_signature)) {
			cout << "there is a problem, please report it" << endl;
			return;
		}

		if (i % jump == 0) {
			cout << i / jump + 1 << " / " << size / jump << endl;
		}
	}

	cout << "all correctness tests passed!" << endl;
}


int main () {
	ios_base::sync_with_stdio(false);
//	cout << "starting random generator test" << endl;
////	random_generator_test(1000000);
//	cout << "starting correctness test" << endl;
//	correctness_test(10000);

	c_crypto_ed25519 ed, ed2;
	ed.generate_key();
	unsigned char Signature[64];
	ed.sign("hello, ed25519 signature, you ulgy bitch", Signature);
	cout << ed2.verify_sign("hello, ed25519 signature, you ulgy bitch", Signature, ed.get_public_key());
	return 0;
}