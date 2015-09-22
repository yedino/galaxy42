#include "c_token.hpp"

size_t token_id_generator::id = 0;

c_chainsign_element::c_chainsign_element (const std::string signed_msg,
		const std::string msg,
		const std::string signer,
		const std::string signer_pubkey) : m_msg(msg), m_signed_msg(signed_msg), m_signer(signer), m_signer_pubkey(signer_pubkey) {
}


c_token::c_token (long long pss) : m_password(pss), id(token_id_generator::generate_id()) { }

bool c_token::check_ps (long long ps) {
	if (ps == m_password) {
		return true;
	}
	return false;
}

long long c_token::get_size() {
	long long size = 0;
	size += sizeof(c_token);
	for(size_t i = 0; i < m_chainsign.size(); ++i) {
		size += sizeof(c_chainsign_element);
		size += m_chainsign[i].m_msg.size();
		size += m_chainsign[i].m_signed_msg.size();
		size += m_chainsign[i].m_signer.size();
		size += m_chainsign[i].m_signer_pubkey.size();
	}
	return size;
}

size_t token_id_generator::generate_id () {
	return id++;
}

bool operator !=(const c_chainsign_element &l_ele, const c_chainsign_element &r_ele) {
	if(	(l_ele.m_signed_msg == r_ele.m_signed_msg) &&
		(l_ele.m_msg == r_ele.m_msg) &&
		(l_ele.m_signer == r_ele.m_signer) &&
		(l_ele.m_signer_pubkey == r_ele.m_signer_pubkey) ) {
		return false;
	} else {
		return true;
	}
}

bool operator== (const c_token &lhs, const c_token &rhs) {
	return lhs.id == rhs.id;
}

bool operator< (const c_token &lhs, const c_token &rhs) {
	return lhs.id < rhs.id;
}

