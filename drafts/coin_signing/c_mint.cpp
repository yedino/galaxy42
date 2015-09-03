#include "c_mint.hpp"

c_token c_mint::emit_token () {
    long long token_pss = generate_password();
    c_token token(token_pss);
    m_emited_tokens.insert( {token, token_pss} );
	return token;
}

bool c_mint::check_isEmited(c_token &tok) {

    if(m_emited_tokens.find(tok) != m_emited_tokens.end() && tok.check_ps(m_emited_tokens[tok])) {
        std::cout << "Token emited here!" << std::endl;
        return true;
    }
    return false;
}

long long c_mint::generate_password () { return random_generator.get_random(sizeof(long long)); }
