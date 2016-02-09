#include "c_token.hpp"

////////////////////////////////////////// CHAINSIGN_ELEMENT //////////////////////////////////////////////////////

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

void c_chainsign_element::print(std::ostream &os) const{

    os << '[' << m_msg << '|' << m_msg_sign <<  '|'
       << m_signer << '|' << m_signer_pubkey << ']' << std::endl;
}

/////////////////////////////////////////////// TOKEN /////////////////////////////////////////////////////////////

size_t token_id_generator::id = 0;

c_token::c_token (const std::string &mintname,const std::string &mint_pubkey, long long password, std::chrono::time_point<std::chrono::system_clock> emit_date, std::chrono::hours exp_time) :
                                        m_mintname(mintname),
                                        m_mint_pubkey(mint_pubkey),
                                        m_password(password),
                                        m_expiration_date(emit_date + exp_time),
                                        m_id(token_id_generator::generate_id())
{ }

void c_token::add_chain_element(const c_chainsign_element &ch) {
    m_chainsign.emplace_back(ch);
}

void c_token::add_chain_element(c_chainsign_element &&ch) noexcept {
    m_chainsign.emplace_back(std::move(ch));
}

const std::vector<c_chainsign_element>& c_token::get_chainsign() const {
    return m_chainsign;
}

const size_t c_token::get_chainsign_size() const {
    return m_chainsign.size();
}

bool c_token::check_ps (long long ps) {
	if (ps == m_password) {
		return true;
	}
	return false;
}

void c_token::print(std::ostream &os, bool verbouse) const {

    std::time_t t = std::chrono::system_clock::to_time_t(m_expiration_date);
    os << "Emiter: [" << m_mintname
       << "], Id: ["  << m_id
       << "], Expiration date: [" << std::ctime(&t)
       << "], Size: [" << get_size()
       << " B]" << std::endl;
    if(verbouse) {
        for(auto &chain_el : m_chainsign)
            chain_el.print(os);
    }
}

size_t c_token::get_id() const {
    return m_id;
}
std::chrono::time_point<std::chrono::system_clock>  c_token::get_expiration_date() const {
    return m_expiration_date;
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

c_token::c_token(std::string packet) {

    // std::cout << "Serialized recieved token :" << packet << std::endl; //dbg
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

std::string c_token::get_emiter_name() const {
    return m_mintname;
}

std::string c_token::get_emiter_pubkey() const {
    return m_mint_pubkey;
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

