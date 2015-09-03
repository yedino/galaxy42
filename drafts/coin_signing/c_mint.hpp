#ifndef C_MINT_H
#define C_MINT_H

#include "c_token.hpp"
#include "../../crypto_ops/crypto/c_random_generator.hpp"
#include <map>

class c_mint {
public:
		c_mint ();

		c_token emit_token ();

private:
		std::map<c_token, long> m_emited_tokens;
};

#endif // C_MINT_H
