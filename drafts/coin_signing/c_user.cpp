#include "c_user.hpp"

c_user::c_user (std::string username) : m_username(username) {
	m_public_key = m_edsigner.get_public_key();
}

void c_user::send_token (c_user &user, size_t amount) {
	if (amount > 1) {
		std::cout << "error! : test version allow to send only 1 token" << std::endl;
		return;
	}

	c_token tok = m_wallet.tokens.back();
	m_wallet.tokens.pop_back();
	std::string msg = std::to_string(tok.id) + std::to_string(amount);
	std::string signed_tok = m_edsigner.sign(msg);
	tok.m_chainsign.push_back(c_chainsign_element(signed_tok, msg, m_public_key));
	user.recieve_token(tok, amount);
}

void c_user::recieve_token (c_token &token, size_t amount) {
	m_mint.check_isEmited(token);
	if (m_edsigner.verify(token.m_chainsign.back().m_signed_msg, token.m_chainsign.back().m_msg, token.m_chainsign.back().m_msg.length(), token.m_chainsign.back().m_signer_pubkey)) {

		std::cout << "token validate : OK" << std::endl;
		m_wallet.tokens.push_back(token);
	} else {
		std::cout << "token validate : BAD_SIGN !!!" << std::endl;
	}
}

void c_user::emit_tokens (size_t t) {
	c_token emitted_token = m_mint.emit_token();
	m_wallet.add_token(emitted_token);
}
