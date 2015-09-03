#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP
#include <cstdlib>

class token_id_generator {
		static size_t id;
public:
		static size_t generate_id ();
};

struct c_token {
		const size_t id;

		c_token ();
};


#endif //COIN_SIGNING_C_TOKEN_HPP
