#include "c_wallet.hpp"

c_wallet::c_wallet ()  { }

c_wallet::c_wallet (const string &filename)  {
    load_from_file(filename);
}

c_wallet::c_wallet (string &&filename) noexcept {
    load_from_file(std::move(filename));
}
size_t c_wallet::amount () const {
    return m_tokens.size();
}

void c_wallet::print_wallet_status(std::ostream &os, bool verbouse) const {
    os << "Amount of tokens in wallet: " << amount() << std::endl;

    for(auto &tok : m_tokens) {
        tok.print(os,verbouse);
    }
}

void c_wallet::add_token (const c_token &token) {
    m_tokens.push_back(token);
}

bool c_wallet::process_token() const {
    if (m_tokens.empty()) {
        std::cerr << "empty wallet! : no token to process" << std::endl;
        return true;
    }
    return false;
}

size_t c_wallet::clean_expired_tokens() {

    size_t expired_amount = 0;
    for(auto it = m_tokens.begin(); it != m_tokens.end();) {
        if(it->get_expiration_date() < std::chrono::system_clock::now()) {
            expired_amount++;
            std::cout << "Wallet: removing deprecated token: ";
            it->print(std::cout);
            it = m_tokens.erase(it);
        }
        else {
            ++it;
        }
    }
    return expired_amount;
}

void c_wallet::remove_token (const c_token &token) {
    m_tokens.remove(token);
}

void c_wallet::save_to_file(const std::string &filename) {

    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << *this;
}

void c_wallet::load_from_file(const std::string &filename) {
  try {
    std::ifstream ifs(filename);

    boost::archive::text_iarchive ia(ifs);
    ia >> *this;
  } catch (std::exception &ec) {
        std::cerr << "Exception opening/reading/closing file :" << ec.what() << std::endl;
  }
}
