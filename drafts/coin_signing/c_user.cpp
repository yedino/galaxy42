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

bool c_user::recieve_token (c_token &token, size_t amount) {
	if (m_mint.check_isEmited(token)) { // is this token emitted by me?
		for (auto &in : used_tokens) { // is this token used?
			if (in == token) {
				std::cout << "token validate : TOKEN_USED !!!" << std::endl;
				find_the_cheater(token, in);
				return false;
			}
		}
		used_tokens.push_back(token);
	}

	for (auto &current_signature : token.m_chainsign) {
		if (!m_edsigner.verify(current_signature.m_signed_msg, current_signature.m_msg, current_signature.m_msg.length(), current_signature.m_signer_pubkey)) {

			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
			return false;
		} else {
			cout << current_signature.m_msg << '\n'; // TODO m_msg is '11' almost always. what is that?!
		}
	}

	m_wallet.tokens.push_back(token);
	std::cout << "token validate : OK" << std::endl;
	return true;
}

void c_user::emit_tokens (size_t amount) {
	c_token emitted_token = m_mint.emit_token();
	m_wallet.add_token(emitted_token);
}

void c_user::find_the_cheater (const c_token &token_a, const c_token &token_b) {
//	for ()
}
