#include "c_user.hpp"

c_user::c_user() :
    public_key(m_edsigner.get_public_key()){
}

void c_user::send_token (c_user &user, size_t amount) {

    if(amount > 1) {
        std::cout << "error! : test version allow to send only 1 token" << std::endl;
        return;
    }
    c_token tok = wallet.tokens.back();
    wallet.tokens.pop_back();
    std::string msg = std::to_string(tok.id) + std::to_string(amount);
    std::string signed_tok = m_edsigner.sign(msg);
    tok.m_chainsign.push_back(std::make_pair(signed_tok,msg));
    user.recieve_token(m_edsigner.get_public_key(), tok, amount);
}

void c_user::recieve_token (std::string pub_key, c_token &token, size_t amount) {
    if(m_edsigner.verify(token.m_chainsign.back().first,
                         token.m_chainsign.back().second,
                         token.m_chainsign.back().second.length(),
                         pub_key)) {
        std::cout << "token validate : OK" << std::endl;
        wallet.tokens.push_back(token);
    } else {
        std::cout << "token validate : BAD_SIGN !!!" << std::endl;
    }
}

void c_user::emit_tokens (size_t t) {
	c_token emitted_token = mint.emit_token();
	wallet.add_token(emitted_token);
}
