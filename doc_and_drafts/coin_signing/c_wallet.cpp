#include "c_wallet.hpp"

c_wallet::c_wallet ()
{ }

c_wallet::c_wallet (const string &filename)  {
    load_from_file(filename);
}

c_wallet::c_wallet (string &&filename) noexcept {
    load_from_file(std::move(filename));
}

bool c_wallet::process_token() const {
    if (m_tokens.empty()) {
        std::cerr << "empty wallet! : no token to process" << std::endl;
        return true;
    }
    return false;
}

c_token c_wallet::get_any_token(bool keep_in_wallet) {

    c_token tok(m_tokens.front());
    if(!keep_in_wallet) {
        m_tokens.pop_front();
    }
    return tok;
}

size_t c_wallet::amount() const {
    return m_tokens.size();
}

void c_wallet::move_token(c_token &&token) {
    std::cout << "BUU" <<std::endl;
    m_tokens.emplace_back(std::move(token));
}

size_t c_wallet::clean_expired_tokens() {

    size_t size_before = m_tokens.size();
    m_tokens.erase(
            std::remove_if(	m_tokens.begin(),
                            m_tokens.end(),
                            [] (const c_token &element) {
                                    if(element.get_expiration_date() < std::chrono::system_clock::now()){
                                        return true;
                                    }
                                    return false;
                            }),
            m_tokens.end());

    size_t expired_amount = size_before - m_tokens.size();
    return expired_amount;
}

void c_wallet::print_wallet_status(std::ostream &os, bool verbouse) const {
    os << "Amount of tokens in wallet: " << amount() << std::endl;

    for(auto &tok : m_tokens) {
        tok.print(os,verbouse);
    }
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

void c_wallet::remove_token (const c_token &token) {
    m_tokens.remove(token);
}
