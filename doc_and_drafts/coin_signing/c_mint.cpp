#include "c_mint.hpp"

////////////////////////////////////////////////////////////////////////////////////////////// ID GENERATOR

token_id_generator::token_id_generator () : id(1)	// 1 instead of 0, becouse 0 is reserved for last expiried id
{ }

size_t token_id_generator::generate_id() {
    return id++;
}

////////////////////////////////////////////////////////////////////////////////////////////// MINT

c_mint::c_mint (const std::string &mintname,
                const std::string &pubkey,
                const std::chrono::seconds &exp_time) :	m_pubkey(pubkey),
                                                        m_mintname(mintname),
                                                        t_expiration_time(exp_time),
                                                        m_last_expired_id(0)
{ }

c_token c_mint::emit_token() {
    size_t t_id = m_id_generator.generate_id();
    long long t_password = generate_password();

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t_expiration_date = now+t_expiration_time;

    c_token token(c_token_header(m_mintname, m_pubkey, t_id, t_password, t_expiration_date));
    m_emited_tokens.insert({token, t_password});
    std::cout << "New token emited:"; token.print(std::cout);

	return token;
}

bool c_mint::check_isEmited(c_token &token) {
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

long long c_mint::generate_password() {
    return random_generator.get_random(sizeof(long long));
}

size_t c_mint::clean_expired_tokens() {

    size_t expired_amount = 0;
    // We must use non pretty loop here, becouse std::remove_if don't working with map
    // This should be correct in c++17
    for(auto it = m_emited_tokens.begin(); it != m_emited_tokens.end();) {
        if(it->first.get_expiration_date() < std::chrono::system_clock::now()) {
            expired_amount++;
            m_last_expired_id = std::max(it->first.get_id(),m_last_expired_id);
            std::cout << "Mint: removing deprecated token: ";
            it->first.print(std::cout);

            it = m_emited_tokens.erase(it);
        }
        else {
            ++it;
        }
    }
    return expired_amount;
}

size_t c_mint::get_last_expired_id() const {
    return m_last_expired_id;
}
