/* See the LICENSE.txt for license information */

#ifndef CRYPTO_C_CRYPTO_GEPORT_H
#define CRYPTO_C_CRYPTO_GEPORT_H
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include "c_random_generator.hpp"

using namespace boost::multiprecision;
using std::string;


template <typename long_type>
struct signed_msg {
		long_type public_key;
		size_t pop_count;
		long_type Signature[256 + 8];
};

template <size_t hash_length, size_t log_hash_length,
	number<cpp_int_backend<hash_length * 2, hash_length * 2, unsigned_magnitude, unchecked, void>> hash_function(const string &)>
class c_crypto_geport {
public:
		typedef number<cpp_int_backend<hash_length * 2, hash_length * 2, unsigned_magnitude, unchecked, void>> long_type;
		static_assert(1 << log_hash_length == hash_length, "invalid lengths provided");

private:
		mutable c_random_generator<long_type> rd_gen;

		static long_type generate_hash (const string &value) { return hash_function(value); }

		static long_type generate_hash (const long_type &value) { return generate_hash(string(value)); }

		static void join_hash (long_type &a, const long_type &b) {
			a <<= 256;
			a += b;
			a = generate_hash(a);
		}

		static size_t bits_counter (long_type value) {
			size_t counter = 0;
			for (size_t i = 0; i < 256; ++i, value >>= 1) {
				if (value & 1)
					++counter;
			}
			return counter;
		}

public:
		c_crypto_geport () = default;

		void generate_private_key (long_type *Private_key) {
			for (size_t i = 0; i < 256 + 8; ++i)
				Private_key[i] = rd_gen.get_random((256 + 8) / 8);
		}

		static signed_msg<long_type> sign (const string &msg, long_type Private_key[256 + 8]) {
			long_type hashed_msg = generate_hash(msg);

			signed_msg<long_type> signature;

			signature.public_key = 0;
			signature.pop_count = bits_counter(hashed_msg);
			long_type hashed_private_key;


			if (signature.pop_count > 256 / 2) {
				hashed_msg = ~hashed_msg;
			}

			for (short i = 0; i < 256; ++i) {
				hashed_private_key = generate_hash(Private_key[i]);
				if (hashed_msg & (1 << i))
					signature.Signature[i] = Private_key[i];
				else
					signature.Signature[i] = hashed_private_key;

				join_hash(signature.public_key, generate_hash(hashed_private_key));
			}

			for (short i = 256; i < 256 + 8; ++i) {
				hashed_private_key = generate_hash(Private_key[i]);
				if (signature.pop_count & (1 << i))
					signature.Signature[i] = Private_key[i];
				else
					signature.Signature[i] = hashed_private_key;

				join_hash(signature.public_key, generate_hash(hashed_private_key));
			}

			return signature;
		}

		static bool verify_sign (const string &msg, const signed_msg<long_type> &signature) {
			long_type hashed_msg = generate_hash(msg), tmp;
			long_type key = 0;
			long_type hashed_signature;

			if (signature.pop_count != bits_counter(hashed_msg))
				return false;


			if (signature.pop_count > 256 / 2) {
				hashed_msg = ~hashed_msg;
			}

			for (short i = 0; i < 256; ++i) {
				hashed_signature = generate_hash(signature.Signature[i]);
				if (hashed_msg & (1 << i))
					tmp = hashed_signature;
				else
					tmp = signature.Signature[i];

				join_hash(key, generate_hash(tmp));
			}

			for (short i = 256; i < 256 + 8; ++i) {
				hashed_signature = generate_hash(signature.Signature[i]);
				if (signature.pop_count & (1 << i))
					tmp = hashed_signature;
				else
					tmp = signature.Signature[i];

				join_hash(key, generate_hash(tmp));
			}
			return (key == signature.public_key);
		}
};


#endif //CRYPTO_C_CRYPTO_GEPORT_H
