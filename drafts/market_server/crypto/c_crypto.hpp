/*
 * WARNING!
 * FOR INNER TEST USE ONLY
 * DO NOT USE IT IN REAL CRYPTOGRAPHY
 * HINT: COMPILE IT WITH -O3
 *       DUE TO LONG-TIME PRIME NUMBER GENERATION
 *
 * C++11 NEEDED
 */


#ifndef RSA__C_CRYPTO_H
#define RSA__C_CRYPTO_H
#include <fstream>
#include "libmath.hpp"
//#include <boost/multiprecision/cpp_int.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/hex.hpp>
#include <iostream>
#include <random>
#include <string>
#include "sha512.hpp"

using namespace boost::multiprecision;
using namespace boost::algorithm;
using std::cout;
using std::cin;
using std::string;

template <typename T>
struct public_key { // TODO
	T e;
	T n;
};

template <typename T>
struct blinded_msg { // TODO
	T msg;
	T blinding_factor;
};

template <size_t len>
class c_crypto {
public:
	typedef number<cpp_int_backend<len * 4, len * 4, signed_magnitude, checked, void>> long_type;
private:
	long_type e, n;

	long_type d, r; // IMPORTANT DATA

	long_type dots_mod;

	long_type gen_rd () const {
		string device;

#ifdef TARGET_OS_MAC
        device = ""; // TODO
#elif defined __linux__
		device = "/dev/urandom"; // TODO change to /dev/random
#elif defined _WIN32 || defined _WIN64
        device = ""; // TODO
#elif defined __ANDROID_API__
        device = ""; // TODO
#endif

		std::ifstream random_generator(device, std::ios_base::in);
		if (!random_generator.is_open())
			throw std::runtime_error("some error occured while reading from random number generator device");

		random_generator.rdbuf()->pubsetbuf(nullptr, 0); // stop buffering data
		long_type random = 0;
		unsigned char readed;
		size_t size_of_random = 0;
		while (size_of_random < len) {
			readed = (unsigned char)random_generator.get();
			random <<= (sizeof(readed) * 8);
			random += (int)readed;
			size_of_random += (sizeof(readed) * 8);
		}
		random_generator.close();
		return random;
	}

	static bool advanced_is_prime (const long_type &p) {
		if (!is_non_prime(p, 5)) {
			if (is_prime(p, 64))
				return true;
		}

		return false;
	}

	long_type generate_random () const { return gen_rd() ^ gen_rd(); }

	long_type generate_prime_number () const {
		long_type random = generate_random();
		size_t dots = 0;
		if (!(random & 1))
			random -= 1;

		cout << "generating prime number";
		cout.flush();
		for (long_type jump = 0; ; jump += 2) {
			if (advanced_is_prime(random + jump)) {
				random += jump;
				break;
			}

			if (advanced_is_prime(random - jump)) {
				random -= jump;
				break;
			}

			if (jump % dots_mod == 0) {
				if (dots == 10) {
					cout << "\b\b\b\b\b\b\b\b\b\b";
					cout << "          ";
					cout << "\b\b\b\b\b\b\b\b\b\b";
					cout.flush();
					dots = 0;
				} else {
					++dots;
					cout << '.';
					cout.flush();
				}
			}
		}
		cout << '\n';
		return random;
	}

	long_type convert_string_to_number (const string &data) const {
		long_type result = 0;
		for (const char &byte : data) {
			result <<= (sizeof(byte) * 8);
			result += (int)byte;
		}
		return result;
	}

	string convert_number_to_string (long_type number) const {
		string result;
		short tmp;
		while (number > 0) {
			tmp = short(number & 255);
			result = char(tmp) + result;
			number >>= 8;
		}
		return result;
	}

	long_type generate_coprime (const long_type &p) const {
		long_type prime;
		do
			prime = generate_prime_number();
		while (gcd(prime, p) != 1);

		return prime;
	}

	long_type generate_less_coprime (const long_type &p) {
		long_type coprime = gen_rd(), div = 2;
		for (long_type org = coprime; coprime >= p; ++div)
			coprime = org / div; // TODO ?

		if (!(coprime & 1))
			coprime -= 1;

		for (long_type jump = 0; gcd(coprime - jump, p) != 1; jump += 2)
			coprime -= jump;

		if (gcd(coprime, p) != 1 || coprime >= p)
			throw std::runtime_error("somoe error while generating coprime, FIX IT BRO");

		return coprime;
	}

public:
	c_crypto () : e(0), n(0), d(0), r(0) {
		double tmp = log2(len);
		if (std::floor(tmp) != tmp)
			throw std::runtime_error("initializing crypto: non power of 2 key length");

		switch (len) {
			case 2048:
				dots_mod = 10;
		    break;
			case 1024:
				dots_mod = 50;
		    break;
			case 512:
				dots_mod = 100;
		    break;
			default:
				dots_mod = 5;
		}
	}

	c_crypto (const c_crypto &rhs) = delete;

	c_crypto (c_crypto &&rhs) = delete;

	c_crypto &operator= (const c_crypto &rhs) = delete;

	c_crypto &operator= (c_crypto &&rhs) = delete;

	string generate_hash (const string &msg) const { return sha512<string>(msg); }

	bool verify_hash (const string &msg, const string &hash) const { return (generate_hash(msg) == hash); }

	void generate_key () {
		if (n != 0 || e != 0 || d != 0) {
			char decision;
			do {
				cout << "Key already generated. Are you sure you want to regenerate it? [y/n] ";
				cin >> decision;
			} while (decision != 'y' && decision != 'Y' && decision != 'n' && decision != 'N');
			if (decision == 'n' || decision == 'N')
				return;
		}

		long_type _p = generate_prime_number();
		long_type _q = generate_prime_number();
		n = _p * _q;
		long_type _totient = euler_func(_p, _q);
		_p = _q = 0;
		e = 65537;
		if (gcd(_totient, e) != 1)
			throw std::runtime_error("gcd not equals 1, TODO"); // TODO

		d = mult_inverse(e, _totient);
		_totient = 0;
		r = generate_prime_number();
	}

	public_key<long_type> get_public_key () const {
		if (e == 0 || n == 0)
			throw std::runtime_error("no public key generated");

		return {e, n};
	}

	long_type encrypt (const string &msg, const long_type &e, const long_type &n) const { // TODO AES symmetric pswd
		long_type converted_msg = convert_string_to_number(msg);
		return power_modulo(converted_msg, e, n); // TODO powm?
	}

	string decrypt (const long_type &msg) const {
		if (d == 0 || n == 0)
			throw std::runtime_error("error while decrypting: please generate your private key first");

		long_type cipher_txt = power_modulo(msg, d, n); // TODO powm?
		return convert_number_to_string(cipher_txt);
	}

	long_type sign (const string &msg) const { // TODO
		if (n == 0 || d == 0)
			throw std::runtime_error("please generate your private key first");

		long_type msg_hash = sha512<long_type>(msg);
		return power_modulo(msg_hash, d, n);
	}

	bool verify_sign (const string &msg, const long_type &sign, const long_type &e, const long_type &n) const { // TODO
		long_type hash = sha512<long_type>(msg);
		return (power_modulo(sign, e, n) == hash);
	}

	blinded_msg<long_type> blind (const string &msg, const long_type &e, const long_type &n) {
		if (r == 0)
			throw std::runtime_error("Please generate your key first");

		if (gcd(r, n) != 1)
			r = generate_coprime(n);

		long_type hash = sha512<long_type>(msg);
		long_type tmp = power_modulo(r, e, n);
		return {mult_modulo(hash, tmp, n), r};
	}

	bool verify_blinded_sign (const string &msg,
		const long_type &blinded_sign,
		const long_type &blinding_factor,
		const long_type &e,
		const long_type &n) const {
		if (r == 0)
			throw std::runtime_error("Please generate your key first");

		long_type r_inv = mult_inverse(blinding_factor, n);
		long_type unblinded_sign = mult_modulo(blinded_sign, r_inv, n);
		long_type hash = sha512<long_type>(msg);
		return (hash == power_modulo(unblinded_sign, e, n));
	}

	long_type blinded_sign (const long_type &blinded_msg) const {
		if (n == 0 || d == 0)
			throw std::runtime_error("Please generate your key first");

		return power_modulo(blinded_msg, d, n);
	}
};


#endif //RSA__C_CRYPTO_H