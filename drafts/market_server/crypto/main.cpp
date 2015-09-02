#include <iostream>
#include "c_crypto.hpp"
#include <boost/algorithm/hex.hpp>
#include <chrono>

using namespace std;

string randomm (int size) {
	static const char alphanum[] = "0123456789"
		"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	static int stringLength = sizeof(alphanum) - 1;
	string result;
	for (int i = 0; i < size; ++i)
		result += alphanum[rand() % stringLength];

	return result;
}

bool auto_encryption_test () {
	c_crypto<2048> to, from;
	to.generate_key();
	string msg;
	auto key = to.get_public_key();
	for (size_t i = 0; i < 1000; ++i) {
		cout << i << '\n';
		msg = randomm(15);
		auto encrypted = from.encrypt(msg, key.e, key.n);
		if (to.decrypt(encrypted) != msg) {
			cout << "DAMN IT\n";
			return false;
		}
	}
	cout << "GREAT!\n";
	return true;
}

void manual_test () {
	c_crypto<2048> to, from;
	to.generate_key();
	string msg;
	while (1) {
		cin >> msg;
		auto encrypted = from.encrypt(msg, to.get_public_key().e, to.get_public_key().n);
		cout << to.decrypt(encrypted) << '\n';
	}
}

bool auto_sign_test () {
	c_crypto<2048> to, from;
	from.generate_key();
	string msg;
	auto key = from.get_public_key();
	for (size_t i = 0; i < 1000; ++i) {
		cout << i << '\n';
		msg = randomm(15);
		auto _signed = from.sign(msg);
		if (to.verify_sign(msg, _signed, key.e, key.n) == false) {
			cout << "DAMN IT\n";
			return false;
		}
	}
	cout << "GREAT!\n";
	return true;
}

void time_measuring () {
	c_crypto<1024> generator;
	c_crypto<1024>::long_type random;
	double steps = 10000;

	auto begin = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < steps; ++i) {
		random = generator.generate_random();
		if (random % 2 == 0)
			++random;

		if (!is_non_prime(random, 3))
			cout << "bingo ";
	}
	auto end = std::chrono::high_resolution_clock::now();
	cout << '\n';
	cout << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " microseconds\n";
}

void effectivity () {
	c_crypto<2048> generator;
	c_crypto<2048>::long_type random;
	double rejected = 0, non_primes = 0;
	double steps = 50000;

	for (int i = 0; i < steps; ++i) {
		random = generator.generate_random();
		if (random % 2 == 0)
			++random;

		if (is_non_prime(random, 1)) {
			++rejected;
			++non_primes;
		} else if (!is_prime(random, 40))
			++non_primes;
	}
	cout << "effectivity: " << rejected / non_primes << '\n' << "rejected: " << rejected << " all non-primes: " <<
	non_primes << '\n';
}

void rejectivity () {
	const size_t size = 512;
	for (int i = 0; i < 20; ++i) {
		c_crypto<size> c;
		c.generate_key();
	}
}

void generating_primes () {
	for (int i = 0; i < 3; ++i) {
		c_crypto<2048> a;
		a.generate_key();
	}
}

bool auto_blinding_test () {
	c_crypto<2048> from, to;
	from.generate_key();
	to.generate_key();
	string msg;
	for (int i = 0; i < 1000; ++i) {
		msg = randomm(10);
		cout << i << '\n';
		auto blinded = from.blind(msg, to.get_public_key().e, to.get_public_key().n);
		auto blinded_sign = to.blinded_sign(blinded.msg);
		if (!from.verify_blinded_sign(msg, blinded_sign, blinded.blinding_factor, to.get_public_key().e, to.get_public_key().n)) {
			cout << "DAMN IT!\n";
			return false;
		}
	}
	cout << "GREAT!\n";
	return true;
}

int main (int argc, char *argv[]) {
	//	c_crypto<2048> c, d;
	//	c.generate_key();
	//	d.generate_key();
	//	ios_base::sync_with_stdio(false);
	//	string msg = "hello, blinding benchmark!";
	//	auto blinded = c.blind(msg, d.get_public_key().e, d.get_public_key().n);
	//	auto begin = std::chrono::high_resolution_clock::now();
	//	volatile size_t res;
	//	for (int i = 0; i < 1000; ++i) {
	//		c.sign(msg);
	//	}
	//
	//	auto end = std::chrono::high_resolution_clock::now();
	//	cout << '\n';
	//	cout << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " microseconds\n";
	c_crypto<8> c;
	unsigned char kr[32];
	for (auto &i : kr)
		i = (unsigned char)(c.gen_rd());
	for (auto &i : kr)
		cout << int(i) << '\n';



	return 0;
}