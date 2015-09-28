#ifndef CONTRACT_SIGNATURE_C_CONTRACT_HPP
#define CONTRACT_SIGNATURE_C_CONTRACT_HPP
#include "c_user.hpp"
#include <string>

using std::string;

class c_contract { // TODO implement operators + constructors
public: // TODO
		const c_user &user_a, &user_b;
		size_t data_size;
		string message;
		unsigned char Signature[64];

		c_contract ();

public:
		c_contract (const c_user &, const c_user &, size_t);

		c_contract (const c_contract &);

		c_contract (c_contract &&);

		void create_msg ();

		bool verify_signature () const;

		void create_signature ();

		string to_string () const;

		static c_contract from_string (const string &);

		friend void swap (c_contract &, c_contract &);

		c_contract &operator= (c_contract);
};


#endif //CONTRACT_SIGNATURE_C_CONTRACT_HPP
