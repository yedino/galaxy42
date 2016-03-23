#include "c_mint.hpp"

////////////////////////////////////////////////////////////////////////////////////////////// ID GENERATOR

token_id_generator::token_id_generator () : m_id(1)	// 1 instead of 0, becouse 0 is reserved for last expiried id
{ }

size_t token_id_generator::generate_id() {
    return m_id++;
}

////////////////////////////////////////////////////////////////////////////////////////////// MINT

c_mint::c_mint (const std::string &mintname,
                const ed_key &pubkey,
                const std::chrono::seconds &exp_time) :	m_pubkey(pubkey),
                                                        m_mintname(mintname),
                                                        t_expiration_time(exp_time),
                                                        m_last_expired_id(0)
{ }

c_token c_mint::emit_token() {
    size_t t_id = m_id_generator.generate_id();
    uint16_t t_count = 0;		// new emited coin should have zero counts
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> t_expiration_date = now+t_expiration_time;

    c_token token(c_token_header(m_mintname,
                                 m_pubkey,
                                 t_id,
                                 t_count,
                                 std::chrono::duration_cast<std::chrono::seconds>(t_expiration_date.time_since_epoch()).count()));

    m_emited_tokens.insert({token, std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count()});
    std::cout << "New token emited:"; token.print(std::cout);

	return token;
}

bool c_mint::check_is_emited(const c_token &token) const {
    auto it = m_emited_tokens.find(token);
    if(it != m_emited_tokens.end() &&
       it->first == token) {		// we must check second time with ==operator, becouse map.find() using only <operator

        std::cout << "Token emited here!" << std::endl;
		return true;
	}
	return false;
}

c_contract c_mint::get_used_token (c_token &token) {

    auto in_it = std::find(m_used_tokens.begin(),m_used_tokens.end(), token);
    if(in_it != m_used_tokens.end()) {

        if(coinsign_evidences::simple_malignant_cheater(token, *in_it, m_mintname)) {
            std::cout << "MINT: USED_TOKEN - found malignant cheater" << std::endl;
            throw coinsign_error(15,"DOUBLE SPENDING - found cheater");
        }
        else if(coinsign_evidences::find_token_cheater(token, *in_it, m_mintname)) {
            std::cout << "MINT: USED_TOKEN - found cheater" << std::endl;
            throw coinsign_error(15,"DOUBLE SPENDING - found cheater");
        }
    }
    std::cout << m_mintname << ": emplace back used token" << std::endl;
    m_used_tokens.emplace_back(std::move(token));

    // TODO

    //auto send_to = token.get_chainsign().back().m_signer;
    // TODO send data
    auto c_header = token.get_contract_header();
    c_contract l_contract = c_contract(c_header.m_contract_info);
    return l_contract;
}

void c_mint::print_mint_status(std::ostream &os) const {
    os << "Mint " << m_mintname << " : [" << m_pubkey <<  ']' << std::endl;
    os << "Tokens emited by mint: " << m_emited_tokens.size() << std::endl;
    for(auto &el : m_emited_tokens) {
        os << "Id: [" << el.first.get_id() << "]" << std::endl;
    }
}

size_t c_mint::clean_expired_emited() {

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

size_t c_mint::clean_expired_used() {

    size_t size_before = m_used_tokens.size();
    size_t last_id = m_last_expired_id;
    m_used_tokens.erase(
            std::remove_if(	m_used_tokens.begin(),
                            m_used_tokens.end(),
                            [&last_id] (const c_token &element) {
                                    if(element.get_id() < last_id){
                                        std::cout << "User : remove deprecated token: " << std::endl;
                                        element.print(std::cout);
                                        return true;
                                    }
                                    return false;
                            }),
            m_used_tokens.end());

    size_t expired_amount = size_before - m_used_tokens.size();
    return expired_amount;
}

size_t c_mint::get_last_expired_id() const {
    return m_last_expired_id;
}
