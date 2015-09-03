#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP
#include <cstdlib>
#include <vector>
#include <iostream>

class token_id_generator {
		static size_t id;
public:
		static size_t generate_id ();
};

struct c_token {
		const size_t id;
        std::vector<std::pair<const std::string, const std::string>> m_chainsign;
		c_token ();
};

bool operator== (const c_token &, const c_token &);

bool operator< (const c_token &, const c_token &);

#endif //COIN_SIGNING_C_TOKEN_HPP
