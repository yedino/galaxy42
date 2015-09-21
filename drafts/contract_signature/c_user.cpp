#include "c_user.hpp"

c_user::c_user (const string &nick) : nickname(nick) {
	if (nickname.find('|') != string::npos) {
		throw runtime_error("nickname is not allowed to contain '|' character");
	}

	crypto_api.generate_key();
}

const unsigned char *const c_user::get_public_key () const {
	return crypto_api.get_public_key();
}

c_user::c_user (string &&nick) {
	nickname.swap(nick);
	if (nickname.find('|') != string::npos) {
		throw runtime_error("nickname is not allowed to contain '|' character");
	}

	crypto_api.generate_key();
}

bool c_user::find_the_cheater (const c_token &token_a, const c_token &token_b) {
	auto len = token_a.m_chainsign.size();

	// [A->B]   [B->C]   [C->D] token_a
	// [A->B]   [B->C]   [C->X] token_b if he cheated at end

	// fast check by size of chainsigns
	if (len != token_b.m_chainsign.size()) {
		std::cout << "bad token : comprared tokend have different size!" << std::endl;
		return false;
	}

	bool is_dbspend = false;
	std::cout << "[TOKEN A]" << "\t[TOKEN B]\n";

	for (auto pos = 0; pos < len; ++pos) {
		auto &current_signature_a = token_a.m_chainsign[pos];
		auto &current_signature_b = token_b.m_chainsign[pos];

		// we need to verify only new token A
		bool ok_sign_a = crypto_api.verify_sign(current_signature_a.m_msg, current_signature_a.m_signature, current_signature_a.m_signer_public_key);
		if (!ok_sign_a) {
			std::cout << "token in validate : BAD_SIGN !!!" << std::endl;
			return false;
		}

		std::cout << "[" << current_signature_a.m_signer << "]" << "\t[" << current_signature_b.m_signer << "]\n" <<
		"   |\t   |\n   V\t   V" << std::endl;
		if (current_signature_a != current_signature_b && !is_dbspend) {
			std::cout << "*** !!! DOUBLE SPENDING detected !!!" << std::endl;
			std::cout << "*** !!! the CHEATER is: " << current_signature_a.m_signer << std::endl;
			is_dbspend = true;
		}
	}
	std::cout << "[" << token_a.m_chainsign[0].m_signer << "]\t[" << token_b.m_chainsign[0].m_signer << "]" << std::endl;
	return true;
}

void c_user::send_token (c_user &user, size_t amount) {
	std::cout << "----send---------------------------------------------------------" << std::endl;
	if (m_wallet.tokens.empty()) {
		std::cout << "empty wallet! : no tokens to send" << std::endl;
		return;
	}

	for (size_t i = 0; i < amount; ++i) {
		c_token tok = m_wallet.tokens.back();
		m_wallet.tokens.pop_back();

		std::string msg = std::to_string(tok.id) + "|" + user.nickname;
		// + std::to_string(amount) ; // the nominal is token's dependent
		unsigned char Signature[64];
		crypto_api.sign(msg, Signature);

		tok.m_chainsign.push_back(c_chainsign_element(Signature, msg, nickname, get_public_key()));
		std::cout << "sending token: " << nickname << " => " << user.nickname << std::endl;
		user.recieve_token(tok); // push this coin to the target user
	}
	std::cout << "----send-end-----------------------------------------------------" << std::endl;
}

void c_user::send_fake_token (c_user &user, size_t amount) {
	if (amount > 1) {
		std::cout << "error! : test version allows to send only 1 token" << std::endl;
		return;
	}
	if (m_wallet.tokens.empty()) {
		std::cout << "empty wallet! : no tokens to send" << std::endl;
		return;
	}

	c_token tok = m_wallet.tokens.back();

	std::string msg = std::to_string(tok.id) + "|" + user.nickname;
	// + std::to_string(amount) ; // the nominal is token's dependent
	unsigned char Signature[64];
	crypto_api.sign(msg, Signature);

	tok.m_chainsign.push_back(c_chainsign_element(Signature, msg, nickname, crypto_api.get_public_key()));
	std::cout << "sending token: " << nickname << " => " << user.nickname << std::endl;
	user.recieve_token(tok); // push this coin to the target user
}

bool c_user::recieve_token (c_token &token) {
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
	bool expected_sender_any = false; // do we expecected sender

	// TODO check if the emitter of coin is good for us, are we at all expecting or interested in such coins
	{
		//		expected_sender = MINT_A;
		//		expected_sender_any = true;
	}

	// [A->B]   [B->C]   [C->D]
	for (auto &current_signature : token.m_chainsign) {
		bool ok_sign = crypto_api.verify_sign(current_signature.m_msg, current_signature.m_signature, current_signature.m_signer_public_key);

		std::string current_sender_in_coin = current_signature.m_signer;
		std::string current_recipient_in_coin;

		std::string delimeter = "|";
		std::size_t found = current_signature.m_msg.find(delimeter) + 1; // +1 to avoid delimeter
		if (found != std::string::npos) {
			current_recipient_in_coin = current_signature.m_msg.substr(found).c_str();
			//print_strBytes(current_signature.m_msg);
		}

		// [B->C] is the current sender B allowed to send,  check for error:
		if ((expected_sender_any) && current_sender_in_coin != expected_sender) {
			std::cout << "expected sender [" << expected_sender << "] vs in-coin sender [" << current_sender_in_coin << "]" <<
			std::endl;
			return false;
		}

		if (!ok_sign) {
			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
			return false;
		}

		expected_sender = current_recipient_in_coin;
		expected_sender_any = true;

		cout << "INFO\n" << "chainsign: " << current_signature.m_msg << "\nwas signed by " << current_signature.m_signer <<
		" with public key: ";
		for (size_t i = 0; i < 64; ++i)
			cout << (int)current_signature.m_signature[i];

		cout << '\n';
	}

	m_wallet.tokens.push_back(token);
	std::cout << "token validate : OK" << std::endl;
	return true;
}

void c_user::emit_tokens (size_t amount) {
	c_token emitted_token = m_mint.emit_token();
	m_wallet.add_token(emitted_token);
}