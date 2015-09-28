/* See the LICENSE.txt for license information */

/*
 * WARNING!
 * FOR INNER TEST USE ONLY
 * DO NOT USE IT IN REAL CRYPTOGRAPHY
 * HINT: COMPILE IT WITH -O3 / -Ofast
 *       DUE TO LONG-TIME PRIME NUMBER GENERATION
 *
 * C++11 NEEDED
 */


#ifndef RSA__C_CRYPTO_H
#define RSA__C_CRYPTO_H
#include <fstream>
#include "libmath.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>
#include <random>
#include <string>
#include "sha_src/sha512.hpp"
#include "sha_src/sha256.hpp"
#include "c_random_generator.hpp"

using namespace boost::multiprecision;
using std::cout;
using std::cin;
using std::string;

template <typename T>
struct public_key { // TODO
		T e;
		T n;
};

template <typename T>
inline bool operator== (const public_key<T> &lhs, const public_key<T> &rhs) {
	return (lhs.e == rhs.e && lhs.n == rhs.n);
}

template <typename T>
inline bool operator!= (const public_key<T> &lhs, const public_key<T> &rhs) {
	return !(lhs == rhs);
}

template <typename T>
struct blinded_msg { // TODO
		T msg;
		T blinding_factor;
};

template <size_t key_size>
class c_crypto_RSA {
public:
		typedef number<cpp_int_backend<key_size * 4, key_size * 4, signed_magnitude, unchecked, void>> long_type;
public: // TODO
		long_type e, n;

		long_type d, r; // IMPORTANT DATA

		long_type dots_mod;

		mutable c_random_generator<long_type> rd_gen;

		bool advanced_is_prime (const long_type &p) const {
			if (!is_non_prime(p, 5)) {
				if (is_prime(p, 64))
					return true;
			}

			return false;
		}

		long_type generate_random () const { return rd_gen.get_random(key_size / 8) ^ rd_gen.get_random(key_size / 8); }

		long_type generate_prime_number () const {
			long_type random = generate_random();
			size_t dots = 0;
			if (!(random & 1))
				random -= 1;

//			cout << "generating prime number";
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

//				if (jump % dots_mod == 0) {
//					if (dots == 10) {
//						cout << "\b\b\b\b\b\b\b\b\b\b";
//						cout << "          ";
//						cout << "\b\b\b\b\b\b\b\b\b\b";
//						cout.flush();
//						dots = 0;
//					} else {
//						++dots;
//						cout << '.';
//						cout.flush();
//					}
//				}
			}
//			cout << '\n';
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
			long_type coprime = rd_gen.get_random(key_size), div = 2;
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

		string generate_hash (const string &msg) const { return sha512<string>(msg); }

		bool verify_hash (const string &msg, const string &hash) const { return (generate_hash(msg) == hash); }

public:
		c_crypto_RSA () : e(0), n(0), d(0), r(0) {
			if (key_size < 256)
				throw std::runtime_error("initializing crypto: please set higher key size");

			double tmp = log2(key_size);
			if (std::floor(tmp) != tmp)
				throw std::runtime_error("initializing crypto: non power of 2 key length");

			switch (key_size) {
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

		void generate_key () {
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

		long_type encrypt (const string &msg, const public_key<long_type> &pub_key) const { // TODO AES symmetric pswd
			long_type converted_msg = convert_string_to_number(msg);
			return power_modulo(converted_msg, pub_key.e, pub_key.n);
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

		bool verify_sign (const string &msg, const long_type &sign, const public_key<long_type> &pub_key) const {
			return (power_modulo(sign, pub_key.e, pub_key.n) == sha512<long_type>(msg));
		}

		blinded_msg<long_type> blind (const string &msg, const public_key<long_type> &pub_key) {
			if (r == 0)
				throw std::runtime_error("Please generate your key first");

			if (gcd(r, pub_key.n) != 1)
				r = generate_coprime(pub_key.n);

			long_type hash = sha512<long_type>(msg);
			long_type tmp = power_modulo(r, pub_key.e, pub_key.n);
			return {mult_modulo(hash, tmp, pub_key.n), r};
		}

		// TODO (verify_blinded_sign) < check if blinding is ok
		bool verify_blinded_sign (const string &msg, const blinded_msg<long_type> &blinded, const public_key<long_type> &pub_key) const {
			long_type r_inv = mult_inverse(blinded.blinding_factor, pub_key.n);
			long_type unblinded_sign = mult_modulo(blinded_sign, r_inv, pub_key.n);
			return (sha512<long_type>(msg) == power_modulo(unblinded_sign, pub_key.e, pub_key.n));
		}

		long_type blinded_sign (const long_type &blinded_msg) const {
			if (n == 0 || d == 0)
				throw std::runtime_error("Please generate your key first");

			return power_modulo(blinded_msg, d, n);
		}
};

#endif //RSA__C_CRYPTO_H
