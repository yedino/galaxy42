#include "c_token.hpp"

////////////////////////////////////////////////////////////////////////////////////////////// TOKEN HEADER

c_token_header::c_token_header (const std::string &mintname,
                                const ed_key &mint_pubkey,
                                const size_t id,
                                const uint64_t expiration_date) : m_mintname(mintname),
                                                                  m_mint_pubkey(mint_pubkey),
                                                                  m_id(id),
                                                                  m_expiration_date(expiration_date)
{ }

void c_token_header::print(std::ostream &os) const {

    std::time_t t = std::chrono::system_clock::to_time_t(cs_utils::u64_to_time(m_expiration_date));
    std::string date(ctime(&t));
    size_t end = date.find_last_of('\n');		// to avoid newline at the end of t
    os << "Emiter: [" << m_mintname
       << "], Mint pubkey [" << m_mint_pubkey
       << "], Id: ["  << m_id
       << "], Expiration date: [" << date.substr(0,end)
       << "]" << std::endl;
}

std::chrono::time_point<std::chrono::system_clock>  c_token_header::get_expiration_date() const {
    return cs_utils::u64_to_time(m_expiration_date);
}
////////////////////////////////////////////////////////////////////////////////////////////// CHAIN ELEMENT

c_chainsign_element::c_chainsign_element (const std::string &msg,
                                          const ed_key &msg_sign,
                                          const std::string &signer,
                                          const ed_key &signer_pubkey) : m_msg(msg),
                                                                         m_msg_sign(msg_sign),
                                                                         m_signer(signer),
                                                                         m_signer_pubkey(signer_pubkey)
{ }

c_chainsign_element::c_chainsign_element (const std::string &packet) {

    std::stringstream ss(packet);
    boost::archive::text_iarchive sa(ss);
    sa >> *this;
}

void c_chainsign_element::print(std::ostream &os) const{

    os << '[' << m_msg << '|' << m_msg_sign <<  '|'
       << m_signer << '|' << m_signer_pubkey << ']' << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////// TOKEN


c_token::c_token (const c_token_header &header) : m_header(header)
{ }

c_token::c_token (c_token_header &&header) : m_header(std::move(header))
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

size_t c_token::get_chainsign_size() const {
    return m_chainsign.size();
}

bool c_token::check_date (uint64_t date) {
    if (date == m_header.m_expiration_date) {
		return true;
	}
	return false;
}

void c_token::print(std::ostream &os, bool verbouse) const {

    m_header.print(os);
    if(verbouse) {
        for(auto &chain_el : m_chainsign) {
            chain_el.print(os);
        }
        os << "Size of token: [" << get_size() << " B]" << std::endl;
    }
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
    return m_header.m_mintname;
}

ed_key c_token::get_emiter_pubkey() const {
    return m_header.m_mint_pubkey;
}

size_t c_token::get_id() const {
    return m_header.m_id;
}

std::chrono::time_point<std::chrono::system_clock>  c_token::get_expiration_date() const {
    return m_header.get_expiration_date();
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

//////////////////////////////////////////////////////////////////////////////////////////////////// JSONCPP

void c_token_header::json_serialize(Json::Value &root) {
    // serialize primitives
    root["mintname"] = m_mintname;
    root["mint_pubkey"] = std::string(reinterpret_cast<const char *>(m_mint_pubkey.c_str()),m_mint_pubkey.size());
    root["id"] = static_cast<Json::UInt64>(m_id);
    root["expiration_date"] = static_cast<Json::UInt64>(m_expiration_date);

}
void c_token_header::json_deserialize(Json::Value &root) {
    // deserialize primitives
    std::cout << "c_token_header: json deserialize [" << root.asString() << "]" << std::endl;
}

void c_chainsign_element::json_serialize(Json::Value &root) {
    // serialize primitives
    root["msg"].append(m_msg);
    root["msg_sign"].append(std::string(reinterpret_cast<const char *>(m_msg_sign.c_str()), m_msg_sign.size()));
    root["signer"].append(m_signer);
    root["signer_pubkey"].append(std::string(reinterpret_cast<const char *>(m_signer_pubkey.c_str()), m_signer_pubkey.size()));
}
void c_chainsign_element::json_deserialize(Json::Value &root) {
    // deserialize primitives
    std::cout << "c_chainsign_element: json deserialize [" << root.asString() << "]" << std::endl;
}

void c_token::json_serialize(Json::Value &root) {
    // serialize primitives
    m_header.json_serialize(root);
    for(auto &chain_el : m_chainsign) {
        chain_el.json_serialize(root);
    }
}
void c_token::json_deserialize(Json::Value &root) {
    // deserialize primitives
    std::cout << "c_token: json deserialize [" << root.asString() << "]" << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// operators

bool operator != (const c_chainsign_element &l_ele, const c_chainsign_element &r_ele) {
    if(	(l_ele.m_msg_sign == r_ele.m_msg_sign) &&
		(l_ele.m_msg == r_ele.m_msg) &&
		(l_ele.m_signer == r_ele.m_signer) &&
		(l_ele.m_signer_pubkey == r_ele.m_signer_pubkey) ) {
		return false;
	} else {
		return true;
	}
}

bool operator == (const c_token &lhs, const c_token &rhs) {
    if(lhs.get_id() != rhs.get_id()) {
        return false;
    }
    if(lhs.get_emiter_pubkey() != rhs.get_emiter_pubkey()) {
        return false;
    }
    if(lhs.get_expiration_date() != rhs.get_expiration_date()) {
        return false;
    }

    return true;
}

bool operator < (const c_token &lhs, const c_token &rhs) {
    return (lhs.get_id() < rhs.get_id());
}
