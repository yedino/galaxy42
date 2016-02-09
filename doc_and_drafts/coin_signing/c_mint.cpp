#include "c_mint.hpp"

c_mint::c_mint (std::string mintname, std::string pubkey, std::chrono::hours exp_time) :
                                                    m_pubkey(pubkey),
                                                    m_mintname(mintname),
                                                    t_expiration_time(exp_time)
{ }

c_token c_mint::emit_token () {
    long long token_pss = generate_password();

    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

    c_token token(m_mintname, m_pubkey, token_pss, start, t_expiration_time);
    m_emited_tokens.insert({token, token_pss});
	return token;
}

bool c_mint::check_isEmited (c_token &token) {
    if (m_emited_tokens.find(token) != m_emited_tokens.end()) {
		std::cout << "Token emited here!" << std::endl;
		return true;
	}
	return false;
}

void c_mint::print_mint_status(std::ostream &os) const {
    os << "Mint " << m_mintname << " : [" << m_pubkey <<  ']' << std::endl;
    os << "Tokens emited by mint: " << m_emited_tokens.size() << std::endl;
    for(auto &el : m_emited_tokens) {
        os << "Id: [" << el.first.get_id() << "]" << std::endl;
    }
}

long long c_mint::generate_password () {
	return random_generator.get_random(sizeof(long long));
}
