#ifndef RING_CRYPTOGRAPHY_C_SIMULATION_H
#define RING_CRYPTOGRAPHY_C_SIMULATION_H

#include "c_ring_shuffle.hpp"
#include "../crypto/c_crypto_RSA.hpp"
#include <vector>
#include <string>

using std::cout;
using std::ostream;
using std::vector;
using std::string;

class c_simulation {
private:
		vector<c_ring_shuffle> users;

		void initialize_shufflers ();

		void tick ();

		void shuffle_msgs (vector<public_key<c_crypto_RSA<256>::long_type>> &);

		c_logger logger;

public:
		c_simulation () = default;

		void add_user ();

		void print_users (std::ostream & = cout);

		void start_simluation ();

		void print_connections (std::ostream & = cout);

		void initialize_simulation ();
};

#endif //RING_CRYPTOGRAPHY_C_SIMULATION_H
