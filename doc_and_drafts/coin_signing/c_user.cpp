#include "c_user.hpp"
void print_strBytes(const std::string& str);

c_user::c_user (std::string& username) : m_username(username) {
	m_public_key = m_edsigner.get_public_key();
	m_reputation = 1;
}

c_user::c_user (string &&nick) {
	m_username.swap(nick);
	if (m_username.find('|') != string::npos) {
		throw runtime_error("nickname is not allowed to contain '|' character");
	}
	m_public_key = m_edsigner.get_public_key();
	m_reputation = 1;
}

string c_user::get_username() const {
	return m_username;
}

string c_user::get_public_key() const{
    return m_public_key;
}

double c_user::get_rep() {
	return atan(m_reputation)*100*(2/M_PI);
}

void c_user::print_used_status(std::ostream &os) {
    os << "Amount of used tokens: " << used_tokens.size() << std::endl;
    for(c_token &tok : used_tokens) {
        os << "Id: [" << tok.get_id() << "], Size: [" << tok.get_size() << "B: ]" << std::endl;
    }
}


c_token c_user::process_token_tosend(const std::string &user_pubkey, bool fake) {
    std::lock_guard<std::mutex> lck (m_mtx);
    if(m_wallet.process_token()) {
        std::string msg = m_username + " can't send token -- transaction abort";
        throw(msg);
    }
    c_token tok = m_wallet.tokens.back(); // take any token for now [TODO]
    if(!fake) {
        m_wallet.tokens.pop_back();
    }
    std::string msg = std::to_string(tok.get_id()) + "|" + user_pubkey;
    std::string msg_sign = m_edsigner.sign(msg);

    tok.m_chainsign.emplace_back(std::move(c_chainsign_element(msg, msg_sign, m_username, m_public_key)));
    return tok;
}

void c_user::send_token_bymethod(c_user &user, bool fake) {
    std::cout << "----send-start-by-method---------------------------------------------------" << std::endl;

  try {
    c_token tok = process_token_tosend(user.get_public_key(),fake);

    std::cout << "sending token: " << m_username << " => " << user.get_username() << std::endl;		// dbg
    user.recieve_token(tok); // push this coin to the target user
    std::cout << "----send-end-----------------------------------------------------" << std::endl;
  } catch(std::string &message) {
    std::cerr << message << std::endl;
  }
}

std::string c_user::get_token_packet(const std::string &user_pubkey, bool fake) {

  try {
    c_token tok = process_token_tosend(user_pubkey,fake);
    m_mtx.lock();
    std::string packet = tok.to_packet();
    m_mtx.unlock();

    return packet;

    } catch(std::string &message) {
    std::cerr << message << std::endl;
    std::string fail = "fail";
    return fail;
  }
}

void c_user::recieve_from_packet(std::string &packet) {
    c_token tok(packet);
    recieve_token(tok);
}

bool c_user::recieve_token (c_token &token) {

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
        bool ok_sign = m_edsigner.verify(current_signature.m_msg_sign,
										 current_signature.m_msg,
										 current_signature.m_signer_pubkey);

		std::string current_sender_in_coin = current_signature.m_signer_pubkey;
		std::string current_recipient_in_coin;

		std::string delimeter = "|";
		std::size_t found = current_signature.m_msg.find(delimeter)+1; // +1 to avoid delimeter
		if (found != std::string::npos) {
			current_recipient_in_coin = current_signature.m_msg.substr(found).c_str();
		}

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

		cout << "INFO\n"
			 << "chainsign: " << current_signature.m_msg << "\nwas signed by " << current_signature.m_signer
			 << " with public key: " << current_signature.m_signer_pubkey << std::endl;
	}

	std::cout << "token validate : OK" << std::endl;
	std::cout << "size of this token : " << token.get_size() << std::endl;

    if (m_mint.check_isEmited(token)) { // is this token emitted by me?
        for (auto &in : used_tokens) { // is this token used?
            if (in == token) {
                std::cout << "token validate : TOKEN_USED !!!" << std::endl;
                if(!find_the_cheater(token, in)) {
                    std::cout << "can't find cheater" << std::endl;
                } else {
                    return false;
                }
            }
        }
        used_tokens.push_back(token);
        return true;		// TODO should we replace token by new one?
    }
    m_wallet.tokens.push_back(token);
    return true;
}

bool c_user::find_the_cheater (const c_token &token_a, const c_token &token_b) {

    auto len_max = std::max(token_a.m_chainsign.size(),token_b.m_chainsign.size());
    auto len_min = std::min(token_a.m_chainsign.size(),token_b.m_chainsign.size());
    bool different_size = (len_max != len_min);
    char bigger = '\0';
    if(different_size) {
        if(token_a.m_chainsign.size() == len_max) {
            bigger = 'a';
        } else {
            bigger = 'b';
        }
    }
	// [A->B]   [B->C]   [C->D] token_a
	// [A->B]   [B->C]   [C->X] token_b if he cheated at end

	bool is_dbspend = false;
	std::cout << "[TOKEN A]" << "\t[TOKEN B]\n";

    for (auto pos=0; pos<len_min; ++pos) {
		auto &current_signature_a = token_a.m_chainsign[pos];
		auto &current_signature_b = token_b.m_chainsign[pos];

		// we need to verify only new token A
        bool ok_sign_a = m_edsigner.verify(current_signature_a.m_msg_sign,
										   current_signature_a.m_msg,
										   current_signature_a.m_signer_pubkey);
		if (!ok_sign_a) {
			std::cout << "token in validate : BAD_SIGN !!!" << std::endl;
			return false;
		}

        std::cout << "[" << current_signature_a.m_signer << "]\t[" << current_signature_b.m_signer << "]\n"
				  << "   |\t   |\n   V\t   V" << std::endl;
		if (current_signature_a != current_signature_b && !is_dbspend) {
			std::cout << "*** !!! DOUBLE SPENDING detected !!!" << std::endl;
			std::cout << "*** !!! the CHEATER is: " << current_signature_a.m_signer << std::endl;
			is_dbspend = true;
		}
    }
    if(different_size == false) {
        std::cout << "[" << token_a.m_chainsign[0].m_signer << "]\t["
                  << token_b.m_chainsign[0].m_signer << "]" << std::endl;
        return true;
    } else if(bigger == 'a') {
        std::cout << "   V\t" << token_b.m_chainsign[0].m_signer << "]" << std::endl;
        for (auto pos=len_min; pos<len_max; ++pos) {
            auto &current_signature_a = token_a.m_chainsign[pos];


            // we need to verify only new token A
            bool ok_sign_a = m_edsigner.verify(current_signature_a.m_msg_sign,
                                               current_signature_a.m_msg,
                                               current_signature_a.m_signer_pubkey);
            if (!ok_sign_a) {
                std::cout << "token in validate : BAD_SIGN !!!" << std::endl;
                return false;
            }
            std::cout << "[" << current_signature_a.m_signer << "]\t" << "\t" << "\n"
                      << "   |\t    \n   V\t    " << std::endl;
        }
        std::cout << "[" << token_a.m_chainsign[0].m_signer << "]\t"
                  << "\t" << std::endl;
        return true;
    } else if(bigger == 'b') {
        std::cout << "[" << token_a.m_chainsign[0].m_signer << "]\t"
                  << "   V" <<  std::endl;
        for (auto pos=len_min; pos<len_max; ++pos) {
            auto &current_signature_b = token_b.m_chainsign[pos];

            std::cout << "\t[" << current_signature_b.m_signer << "]\n"
                      << "   \t   |\n    \t   V" << std::endl;
        }
        std::cout << "\t[" << token_b.m_chainsign[0].m_signer << "]" << std::endl;
        return true;
    } else {
        throw std::string("find_the_cheater: unexpected error");
    }
}

void c_user::emit_tokens (size_t amount) {
	for(size_t i = 0; i < amount; ++i) {
		c_token emitted_token = m_mint.emit_token();
		m_wallet.add_token(emitted_token);
        std::cout << m_username << " emited token with id: "  << emitted_token.get_id() << std::endl;
    }
}

void c_user::print_status(std::ostream &os) {
    os << "|||<<>> " << m_username << " <<>>||| status info:" << std::endl;
    m_mint.print_mint_status(os);
    print_used_status(os);
    m_wallet.print_wallet_status(os,m_username);
}

void print_strBytes(const std::string& str) {
	for(size_t i = 0; i < str.length(); ++i) {
		std::cout << static_cast<int>(str[i]) << ":";
	}
	std::cout << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////// bitwallet

bool c_user::check_bitwallet() {
    return m_bitwallet.is_set();
}

void c_user::set_bitwallet(std::string rpc_username, std::string rpc_password, std::string rpc_address, int rpc_port) {

    m_bitwallet.set_wallet(rpc_username, rpc_password, rpc_address, rpc_port);
}
double c_user::get_bitwallet_balance() {
    return get_bitwallet_balance();
}
