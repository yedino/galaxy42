#include "c_token.hpp"

size_t token_id_generator::id = 0;

c_chainsign_element::c_chainsign_element (const std::string msg,
        const std::string msg_sign,
		const std::string signer,
        const std::string signer_pubkey) : m_msg(msg), m_msg_sign(msg_sign), m_signer(signer), m_signer_pubkey(signer_pubkey) {
}
c_chainsign_element::c_chainsign_element (const std::string &packet) {

    char chain_dl = '&';		// chain element delimeter
    if(std::count(packet.begin(), packet.end(), chain_dl) != 3) {
        throw(std::string("abort: invalid token -- bad chainsign element format!"));
    }

    std::vector<std::string> parsed;
    size_t pos_in_begin = 0;		// begin positon of component in chainelement
    size_t pos_in_end = 0;  			// end positon of component in chainelement
    int i;
    for(i = 0; (pos_in_end = packet.find(chain_dl,pos_in_begin)) != std::string::npos; i++) {
        parsed.push_back(packet.substr(pos_in_begin,pos_in_end - pos_in_begin));
        pos_in_begin = pos_in_end+1;
    }
    parsed.push_back(packet.substr(pos_in_begin,pos_in_end- pos_in_begin));
    m_msg 			= parsed.at(0);
    m_msg_sign 		= parsed.at(1);
    m_signer 		= parsed.at(2);
    m_signer_pubkey = parsed.at(3);
}

c_token::c_token (long long pss) : m_password(pss), id(token_id_generator::generate_id()) { }

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
    return id;
}

c_token::c_token(std::string packet){

    // std::cout << "deserialization: [" << packet << "]" << std::endl; //dbg
    char token_dl = '$';		// token delimenter
    char id_dl = '|';		// id deleimeter
    char pass_dl = '#';	// password delimeter
    size_t chain_el_begin = 0;		// chain element begin
    size_t chain_el_end = 0;		// chain element end
    id = std::numeric_limits<size_t>::max();
    while((chain_el_begin = packet.find(token_dl)) != std::string::npos) {
        std::string chain_element;
        chain_el_end = packet.find(token_dl,chain_el_begin+1);
        size_t pass_block_pos = packet.find(pass_dl,chain_el_begin+1);	// position of last block in packet
        if(pass_block_pos < chain_el_end) {								// that is token password
            chain_element = packet.substr(chain_el_begin+1,pass_block_pos-1);
            chain_el_begin == std::string::npos; 	// last loop iteration
            packet = packet.substr(pass_block_pos);
        } else {
            chain_element = packet.substr(chain_el_begin+1,chain_el_end-1);
            packet = packet.substr(chain_el_end);
        }
        std::size_t id_block_pos = chain_element.find(id_dl); // +1 to avoid delimeter
        if (id_block_pos != std::string::npos && id_block_pos != 0) {
            size_t next_id;
            sscanf(chain_element.substr(0,id_block_pos).c_str(), "%zu", &next_id);
            if(id != std::numeric_limits<size_t>::max() && id != next_id) {
                throw(std::string("bad chainsigns -- invaild token ids"));
            }
            id = next_id;
        }
        else {
            throw(std::string("bad chainsigns -- tokens with missing id"));
        }
        m_chainsign.push_back(chain_element);
    }
    m_password = atoll(packet.substr(1).c_str());
}

std::string c_token::to_packet() {

    std::string packet;
    for(c_chainsign_element& chain_el : m_chainsign) {
        packet += "$";
        packet += chain_el.m_msg + '&';
        packet += chain_el.m_msg_sign + '&';
        packet += chain_el.m_signer + '&';
        packet += chain_el.m_signer_pubkey;
    }
    //TODO adding password should be encrypted!
    packet += "#";
    packet += std::to_string(m_password);
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

