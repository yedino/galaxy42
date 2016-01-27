#include "c_wallet.hpp"

c_wallet::c_wallet (const string &name) : tokens_type(name) { }

c_wallet::c_wallet (string &&name) : tokens_type(name) { }

size_t c_wallet::amount () {
	return tokens.size();
}
void c_wallet::print_wallet_status(std::ostream &os, std::string &my_username) {
    os << "Amount of tokens in wallet: " << amount() << std::endl;

    for(c_token &tok : tokens) {
        std::string emiter;
        (tok.get_emiter_name() != "unknown") ? emiter = tok.get_emiter_name() : emiter = my_username;
        os << "Emiter: [" << emiter << "], Id: ["  << tok.get_id() << "], Size: [" << tok.get_size() << " B]" << std::endl;
    }
}

void c_wallet::add_token (const c_token &token) {
	tokens.push_back(token);
}

bool c_wallet::process_token() const {
    if (tokens.empty()) {
        std::cerr << "empty wallet! : no token to process" << std::endl;
        return true;
    }
    return false;
}


void c_wallet::remove_token (const c_token &token) {
	tokens.remove(token);
}

