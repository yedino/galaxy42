#include "c_token.hpp"

size_t token_id_generator::id = 0;

c_chainsign_element::c_chainsign_element (const std::string signed_msg,
	const std::string msg,
	const std::string signer_pubkey) : m_signed_msg(signed_msg), m_msg(msg), m_signer_pubkey(signer_pubkey) {
}

c_token::c_token (long long pss) : m_password(pss), id(token_id_generator::generate_id()) {
}

bool c_token::check_ps (long long ps) {
	if (ps == m_password) {
		return true;
	}
	return false;
}

size_t token_id_generator::generate_id () { return id++; }

bool operator== (const c_token &lhs, const c_token &rhs) { return lhs.id == rhs.id; }

bool operator< (const c_token &lhs, const c_token &rhs) { return lhs.id < rhs.id; }
