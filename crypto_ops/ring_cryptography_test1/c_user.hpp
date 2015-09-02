#ifndef RING_CRYPTOGRAPHY_C_RING_USER_HPP
#define RING_CRYPTOGRAPHY_C_RING_USER_HPP
#include <string>
#include <vector>
#include "../crypto/c_crypto_RSA.hpp"

using std::string;
using std::vector;

class c_user {
public:
		typedef string t_user;
		typedef public_key<c_crypto_RSA<256>::long_type> t_public_key;
		t_user name;
		t_public_key pub_key;
		vector<t_public_key> inbox;
		c_user () : name(t_user()), pub_key(t_public_key()) { }
};

bool operator== (const c_user &, const c_user &);

bool operator!= (const c_user &, const c_user &);

#endif //RING_CRYPTOGRAPHY_C_RING_USER_HPP
