#ifndef RING_CRYPTOGRAPHY_TMP_C_LOGGER_H
#define RING_CRYPTOGRAPHY_TMP_C_LOGGER_H
#include <string>
#include <iostream>
#include "c_user.hpp"
#include <vector>
#include "../crypto/c_crypto_RSA.hpp"

using std::string;
using std::vector;
using std::cout;
using std::ostream;

class c_logger {
private:
		ostream &output;
public:
		c_logger (ostream &stream = cout);
		void send (c_user &, c_user &, const vector<public_key<c_crypto_RSA<256>::long_type>> &);
};


#endif //RING_CRYPTOGRAPHY_TMP_C_LOGGER_H
