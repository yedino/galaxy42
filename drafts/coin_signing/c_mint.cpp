#include "c_mint.hpp"

long generate_pass () {
	return 12345;
}

c_mint::c_mint () { }

c_token c_mint::emit_token () {
	c_token token;
	m_emited_tokens.insert( {token, generate_pass()} );
	return token;
}
