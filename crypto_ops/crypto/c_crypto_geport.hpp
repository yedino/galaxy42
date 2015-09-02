/* See the LICENSE.txt for license information */

#ifndef CRYPTO_C_CRYPTO_GEPORT_H
#define CRYPTO_C_CRYPTO_GEPORT_H
#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include "sha_src/sha256.hpp"
#include "c_random_generator.hpp"

using namespace boost::multiprecision;
using std::string;

struct signed_msg {
		typedef number<cpp_int_backend<512, 512, unsigned_magnitude, unchecked, void>> long_type;
		long_type public_key;
		long_type Signature[256 + 8];
};


class c_crypto_geport {
public:
		typedef number<cpp_int_backend<512, 512, unsigned_magnitude, unchecked, void>> long_type;

private:
		mutable c_random_generator<long_type> rd_gen;

		static void join_hash (long_type &, const long_type &);

public:
		c_crypto_geport () = default;

		void generate_private_key (long_type[256 + 8]);

		static signed_msg sign (const string &, long_type [256 + 8]);

		static bool verify_sign (const string &, const signed_msg &);
};


#endif //CRYPTO_C_CRYPTO_GEPORT_H
