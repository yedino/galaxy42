#include "c_user.hpp"

c_user::c_user() :
    public_key(m_edsigner.get_public_key()){
}

void c_user::send_token (c_user &user, size_t amount) {

    c_token tok = wallet.tokens.back();
    wallet.tokens.pop_back();
    std::string msg = std::to_string(tok.id) + std::to_string(amount);
    std::string signed_tok = m_edsigner.sign(msg);
    tok.m_chainsign.push_back(std::make_pair(msg,signed_tok));

    user.recieve_token(m_edsigner.get_public_key(), tok, amount);


    std::cout << signed_tok << std::endl;
    std::cout << "is sign valid? : " << m_edsigner.verify(signed_tok,msg,msg.length(),m_edsigner.get_public_key()) << std::endl;
}

void c_user::recieve_token (std::string pub_key, c_token token, size_t amount) {

}

void c_user::emit_tokens (size_t t) {
	c_token emitted_token = mint.emit_token();
	wallet.add_token(emitted_token);
}
