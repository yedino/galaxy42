#include "c_mint.hpp"

c_token c_mint::emit_token () {
	c_token token;
	m_emited_tokens.insert( {token, generate_password()} );
	return token;
}

long long c_mint::generate_password () { return random_generator.get_random(sizeof(long long)); }