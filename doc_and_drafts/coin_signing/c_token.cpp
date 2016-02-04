#include "c_token.hpp"

size_t token_id_generator::id = 0;

c_chainsign_element::c_chainsign_element (const std::string msg,
        const std::string msg_sign,
		const std::string signer,
        const std::string signer_pubkey) : m_msg(msg), m_msg_sign(msg_sign), m_signer(signer), m_signer_pubkey(signer_pubkey) {
}
c_chainsign_element::c_chainsign_element (const std::string &packet) {

    std::stringstream ss(packet);
    boost::archive::text_iarchive sa(ss);
    sa >> *this;
}

c_token::c_token (long long pss) : m_password(pss), m_id(token_id_generator::generate_id()) { }

bool c_token::check_ps (long long ps) {
	if (ps == m_password) {
		return true;
	}
	return false;
}

long long c_token::get_size() const {
    long long size = 0;
	size += sizeof(c_token);
	for(size_t i = 0; i < m_chainsign.size(); ++i) {
		size += sizeof(c_chainsign_element);
		size += m_chainsign[i].m_msg.size();
        size += m_chainsign[i].m_msg_sign.size();
		size += m_chainsign[i].m_signer.size();
		size += m_chainsign[i].m_signer_pubkey.size();
	}
	return size;
}
size_t c_token::get_id() const {
    return m_id;
}

c_token::c_token(std::string packet) {

    std::stringstream ss(packet);
    boost::archive::text_iarchive sa(ss);
    sa >> *this;
}

std::string c_token::to_packet() {

    std::stringstream ss;
    boost::archive::text_oarchive sa(ss);
    sa << *this;

    std::string packet;
    packet = ss.str();
    return packet;
}

std::string c_token::get_emiter_name() {

    if(!m_chainsign.empty()) {
        return m_chainsign[0].m_signer;
    } else {
        return std::string("unknown");
    }

}

size_t token_id_generator::generate_id () {
	return id++;
}

bool operator !=(const c_chainsign_element &l_ele, const c_chainsign_element &r_ele) {
    if(	(l_ele.m_msg_sign == r_ele.m_msg_sign) &&
		(l_ele.m_msg == r_ele.m_msg) &&
		(l_ele.m_signer == r_ele.m_signer) &&
		(l_ele.m_signer_pubkey == r_ele.m_signer_pubkey) ) {
		return false;
	} else {
		return true;
	}
}

bool operator== (const c_token &lhs, const c_token &rhs) {
    return lhs.get_id() == rhs.get_id();
}

bool operator< (const c_token &lhs, const c_token &rhs) {
    return lhs.get_id() < rhs.get_id();
}

