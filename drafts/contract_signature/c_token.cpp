#include "c_token.hpp"

size_t token_id_generator::id = 0;

size_t token_id_generator::generate_id () {
	return id++;
}

c_chainsign_element::c_chainsign_element (const unsigned char *const signed_msg,
	const std::string msg,
	const std::string signer,
	const unsigned char *const signer_pubkey) : m_msg(msg), m_signer(signer) {
	std::copy(signed_msg, signed_msg + 64, m_signature);
	std::copy(signer_pubkey, signer_pubkey + 32, m_signer_public_key);
}

bool operator!= (const c_chainsign_element &lhs, const c_chainsign_element &rhs) {
	for (size_t i = 0; i < 64; ++i) {
		if (lhs.m_signature[i] != rhs.m_signature[i])
			return true;
	}

	for (size_t i = 0; i < 32; ++i) {
		if (lhs.m_signer_public_key[i] != rhs.m_signer_public_key[i])
			return true;
	}
	return false;
}


c_token::c_token (long long pss) : m_password(pss), id(token_id_generator::generate_id()) { }

bool c_token::check_ps (long long ps) {
	return ps == m_password;
}

bool operator== (const c_token &lhs, const c_token &rhs) {
	return lhs.id == rhs.id;
}

bool operator< (const c_token &lhs, const c_token &rhs) {
	return lhs.id < rhs.id;
}

c_token::c_token (const c_token &token) : id(token.id), m_chainsign(token.m_chainsign) { }
