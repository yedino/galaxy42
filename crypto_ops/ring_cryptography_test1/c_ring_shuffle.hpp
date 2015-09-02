#ifndef RING_CRYPTOGRAPHY_C_RING_HPP
#define RING_CRYPTOGRAPHY_C_RING_HPP

#include "c_user.hpp"
#include <list>
#include <algorithm>
#include <initializer_list>
#include "c_logger.hpp"
#include <memory>

using std::initializer_list;
using std::list;

class c_ring_shuffle {
public: // TODO
		void synchronize_rings ();

		void set_my_place_in_group ();

		void shift ();

		void generate_new_identity ();

		void set_center_user();

		list<c_user> ring;

		list<c_user>::iterator next_user, prev_user, center_user;

		c_user current_user;

		c_crypto_RSA<256> new_identity;

		c_crypto_RSA<256> encryption_driver;

		c_logger network_driver;

		c_crypto_RSA<256>::long_type encrypt_msg (const string &);

		list<c_user>::iterator next(const list<c_user>::iterator &); // TODO implement fucking circular list

		list<c_user>::iterator prev(const list<c_user>::iterator &); // TODO implement fucking circular list

public:
		c_ring_shuffle (const c_user &user);

		void initialize_shuffle ();

		void start_shuffle ();

		friend void swap (c_ring_shuffle &, c_ring_shuffle &);

		void add_user (const c_user &);

		void add_user (c_user &&);
};


#endif //RING_CRYPTOGRAPHY_C_RING_HPP
