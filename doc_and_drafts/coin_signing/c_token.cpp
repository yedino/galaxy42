#include "c_token.hpp"

////////////////////////////////////////////////////////////////////////////////////////////// TOKEN HEADER

c_token_header::c_token_header (const std::string &mintname,
                                const ed_key &mint_pubkey,
                                const size_t id,
                                const uint16_t count,
                                const uint64_t expiration_date) : m_mintname(mintname),
                                                                  m_mint_pubkey(mint_pubkey),
                                                                  m_id(id),
                                                                  m_count(count),
                                                                  m_expiration_date(expiration_date)
{ }

void c_token_header::print(std::ostream &os) const {

    std::time_t t = std::chrono::system_clock::to_time_t(cs_utils::u64_to_time(m_expiration_date));
    std::string date(ctime(&t));
    size_t end = date.find_last_of('\n');		// to avoid newline at the end of t
    os << "Emiter: [" << m_mintname
       << "], Mint pubkey [" << m_mint_pubkey
       << "], Id: ["  << m_id
       << "], Counts: ["  << m_count
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


c_token::c_token(const std::string &packet, serialization method) {

    if(method == serialization::boost) {	// boost::serialization way
        std::cout << "Serialized recieved token :" << packet << std::endl; //dbg
        std::stringstream ss(packet);
        boost::archive::text_iarchive sa(ss);
        sa >> *this;
    }
    else if(method == serialization::Json) {// Json::valus way
        c_json_serializer::deserialize(*this, packet);
    }
}

std::string c_token::to_packet(serialization method) {

    std::string packet;
    if(method == serialization::boost) {// boost::serialize way
        unsigned version = boost::archive::BOOST_ARCHIVE_VERSION();
        std::cout << "Serialize token with boost::archive version : " << version << std::endl;	//dbg
        std::stringstream ss;
        boost::archive::text_oarchive sa(ss);
        sa << *this;
        packet = ss.str();
    }
    else if(method == serialization::Json) {// json::value
        c_json_serializer::serialize(*this, packet);
    }
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

uint16_t c_token::get_count () const {
    return m_header.m_count;
}

std::chrono::time_point<std::chrono::system_clock>  c_token::get_expiration_date() const {
    return m_header.get_expiration_date();
}

void c_token::increment_count() {
    m_header.m_count++;
    if(m_header.m_count == std::numeric_limits<uint16_t>::max()) {
        throw std::logic_error("token count overflow!");
    }
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

void c_token_header::json_serialize(Json::Value &root) const {
    // serialize primitives
    root["mintname"] = m_mintname;
    root["mint_pubkey"] = std::string(reinterpret_cast<const char *>(m_mint_pubkey.c_str()),m_mint_pubkey.size());
    root["id"] = static_cast<Json::UInt64>(m_id);
    root["count"] = static_cast<Json::UInt>(m_count);
    root["expiration_date"] = static_cast<Json::UInt64>(m_expiration_date);
}

void c_token_header::json_deserialize(Json::Value &root) {
    // deserialize primitives
    std::cout << "c_token_header: json deserialize [" << root.asString() << "]" << std::endl;
}

void c_chainsign_element::json_serialize(Json::Value &root) const {
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

void c_token::json_serialize(Json::Value &root) const {
    // serialize primitives
    m_header.json_serialize(root);
    for(auto &chain_el : m_chainsign) {
        chain_el.json_serialize(root);
    }
}

// reinterpret_cast using in this fun is safe becouse of:
// cpp reference 5.2.10/7 - Reinterpret cast [expr.reinterpret.cast]
// cpp reference 3.10/10 -
void c_token::json_deserialize(Json::Value &root) {
    // deserialize primitives

    std::string mintname = root.get("mintname", "").asString();
    ed_key mint_pubkey(reinterpret_cast<const unsigned char*>(root.get("mint_pubkey", "").asCString()),crypto_ed25519::public_key_size);
    size_t id = root.get("id",0).asUInt64();
    uint16_t count = root.get("count",std::numeric_limits<uint16_t>::max()).asUInt();
    uint64_t expiration_date = root.get("expiration_date",0).asUInt64();

    std::cout << "json deserialize : " 	<< mintname << ' ' << mint_pubkey << ' '
                                        << id << ' ' << expiration_date << std::endl;
    // TODO better errors
    if(mintname.size() == 0) {
        throw std::logic_error("Bad Json format for c_token : invaild mintname");
    }
    if(mint_pubkey.size() == 0) {
        throw std::logic_error("Bad Json format for c_token : invaild mint_pubkey");
    }
    if(id == 0) {
        throw std::logic_error("Bad Json format for c_token : invaild id");
    }
    if(count == std::numeric_limits<uint16_t>::max()) {
        throw std::logic_error("Bad Json format for c_token : invaild count");
    }
    if(expiration_date == 0) {
        throw std::logic_error("Bad Json format for c_token : invaild expiration_date");
    }

    // dbg
    //std::cout << "c_token: json deserialize mintname [" << mintname << "]" << std::endl;
    //std::cout << "c_token: json deserialize mint_pubkey [" << mint_pubkey << "]" << std::endl;
    //std::cout << "c_token: json deserialize id [" << id << "]" << std::endl;
    //std::cout << "c_token: json deserialize expiration_date [" << expiration_date << "]" << std::endl;

    m_header = c_token_header(mintname,
                              mint_pubkey,
                              id,
                              count,
                              expiration_date);

    if(root.get("msg","").isArray()) {
        Json::Value msg = root.get("msg","");
        Json::Value msg_sign = root.get("msg_sign","");
        Json::Value signer = root.get("signer","");
        Json::Value signer_pubkey = root.get("signer_pubkey","");

        if(!(msg.size() == msg_sign.size() && msg.size() == signer.size() && signer.size() == signer_pubkey.size())) {
            std::cout << msg.size() << std::endl;
            std::cout << msg_sign.size() << std::endl;
            std::cout << signer.size() << std::endl;
            std::cout << signer_pubkey.size() << std::endl;
            throw std::logic_error("Bad Json format for c_token : invaild sizes");
        }

        auto chain_size = root.get("msg","").size();
        for(unsigned int i = 0; i < chain_size; ++i) {
            std::string i_msg = msg[i].asString();
            ed_key i_msg_sign(reinterpret_cast<const unsigned char*>(msg_sign[i].asCString()),crypto_ed25519::signature_size);
            std::string i_signer = signer[i].asString();
            ed_key i_signer_pubkey(reinterpret_cast<const unsigned char*>(signer_pubkey[i].asCString()),crypto_ed25519::public_key_size);

            this->add_chain_element(c_chainsign_element(i_msg,i_msg_sign,i_signer,i_signer_pubkey));
        }
    }
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
