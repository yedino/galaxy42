#include "c_user.hpp"

c_user::c_user (std::string username) : m_username(username) {
	m_public_key = m_edsigner.get_public_key();
}

void c_user::send_token (c_user &user, size_t amount) {
	if (amount > 1) {
		std::cout << "error! : test version allow to send only 1 token" << std::endl;
		return;
	}

	c_token tok = m_wallet.tokens.back(); // take any token for now [TODO]
	// TODO assert if any [UB]

	m_wallet.tokens.pop_back();

	std::string msg = std::to_string(tok.id) + "|" + user.publickey;
	// + std::to_string(amount) ; // the nominal is token's dependent
	std::string signed_tok = m_edsigner.sign(msg);

	tok.m_chainsign.push_back(c_chainsign_element(signed_tok, msg, m_public_key));
	user.recieve_token(tok, amount); // push this coin to the target user
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

	// check validity of the signatures chain

	std::string expected_sender; // publickey
	bool expected_sender_any=false; // do we expecected sender

	// TODO check if the emitter of coin is good for us, are we at all expecting or interested in such coins
	{
		//		expected_sender = MINT_A; 
		//		expected_sender_any = true;
	}

	// [A->B]   [B->C]   [C->D]
	for (auto &current_signature : token.m_chainsign) {

		// [B->C]
		bool ok_sign = m_edsigner.verify(current_signature.m_signed_msg, current_signature.m_msg, 
			current_signature.m_msg.length(), current_signature.m_signer_pubkey);

		auto current_sender_in_coin = current_signature.m_signer_pubkey;
		auto current_recipient_in_coin = "A"; // TODO strpos substr - by delimiter

		// [B->C] is the current sender B allowed to send,  check for error:
		if ( (expected_sender_any) && current_sender_in_coin != expected_sender) {
			std::cout << "expected sender ["<<expected_sender<<"] vs in-coin sender ["<<current_sender_in_coin<<"]" << std::endl;
			return false;
		}

		if (!ok_sign) {
			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
			return false;
		} 

		expected_sender = current_recipient_in_coin;
		expected_sender_any = true;

		cout << current_signature.m_msg << '\n'; // TODO info
	}

	m_wallet.tokens.push_back(token);
	std::cout << "token validate : OK" << std::endl;
	return true;
}

void c_user::find_the_cheater (const c_token &token_a, const c_token &token_b) {

	auto len = token_a.m_chainsign;

	// [A->B]   [B->C]   [C->D] token_a
	// [A->B]   [B->C]   [C->X] token_b if he cheated at end

	for (auto pos=0; pos<len; ++pos) {
		auto &current_signature_a = token_a.m_chainsign[pos];
		auto &current_signature_b = token_b.m_chainsign[pos];

		// [B->C]
		bool ok_sign_a = m_edsigner.verify(current_signature_a.m_signed_msg, current_signature_a.m_msg, 
			current_signature_a.m_msg.length(), current_signature_a.m_signer_pubkey);
		if (!ok_sign_a) {
			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
			return false;
		} 
		
		// TODO deduplicate  for + const auto& = ? .. : ...
		// [B->C]
		bool ok_sign_b = m_edsigner.verify(current_signature_b.m_signed_msg, current_signature_b.m_msg, 
			current_signature_b.m_msg.length(), current_signature_b.m_signer_pubkey);
		if (!ok_sign_b) {
			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
			return false;
		} 

		auto current_sender_in_coin_a = current_signature_a.m_signer_pubkey;
		auto current_recipient_in_coin_a = "A"; // TODO strpos substr - by delimiter
		auto current_sender_in_coin_b = current_signature_a.m_signer_pubkey;
		auto current_recipient_in_coin_b = "A"; // TODO strpos substr - by delimiter

		// [B->C] is the current sender B allowed to send,  check for error:
			if ( (expected_sender_any) && current_sender_in_coin != expected_sender) {
				std::cout << "expected sender ["<<expected_sender<<"] vs in-coin sender ["<<current_sender_in_coin<<"]" << std::endl;
				return false;
			}
			expected_sender = current_recipient_in_coin;
			expected_sender_any = true;
		// TODO x2 for _a and _b

		if (current_recipient_in_coin_a != current_recipient_in_coin_b) {
			std::cout << "DOUBLE SPENDING detected: " << std::endl; // TODO details here
		}

		cout << current_signature.m_msg << '\n'; // TODO info
	}

}


void c_user::emit_tokens (size_t amount) {
	c_token emitted_token = m_mint.emit_token();
	m_wallet.add_token(emitted_token);
}



