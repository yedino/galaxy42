#include "c_user.hpp"
void print_strBytes(const std::string& str);

c_user::c_user (std::string& username) :
    m_evidences(m_edsigner),
    m_mint(username,get_public_key())	// mintname could by other than username
{
    m_username.swap(username);
    if (m_username.find('|') != string::npos) {
        throw std::runtime_error("nickname is not allowed to contain '|' character");
    }
    m_reputation = 1;
}

c_user::c_user (string &&username) :
    m_evidences(m_edsigner),
    m_mint(username,get_public_key())	// mintname could by other than username
{
    m_username.swap(username);
	if (m_username.find('|') != string::npos) {
        throw std::runtime_error("nickname is not allowed to contain '|' character");
	}
    m_reputation = 1;
}

string c_user::get_username() const {
	return m_username;
}

string c_user::get_public_key() const{
    return m_edsigner.get_public_key();
}

double c_user::get_rep() {
	return atan(m_reputation)*100*(2/M_PI);
}

void c_user::print_used_status(std::ostream &os) const {
    os << "Amount of used tokens: " << m_used_tokens.size() << std::endl;
    for(auto &tok : m_used_tokens) {
        os << "Id: [" << tok.get_id() << "], Size: [" << tok.get_size() << "B: ]" << std::endl;
    }
}


c_token c_user::process_token_tosend(const std::string &user_pubkey, bool keep_in_wallet) {
    std::lock_guard<std::mutex> lck (m_mtx);
    if(m_wallet.process_token()) {
        std::string msg = m_username + " can't send token -- transaction abort";
        throw std::logic_error(msg);
    }
    c_token tok = m_wallet.get_any_token(keep_in_wallet); // take any token for now [TODO]

    std::string msg = std::to_string(tok.get_id()) + "|" + user_pubkey;
    std::string msg_sign = m_edsigner.sign(msg);

    tok.add_chain_element(c_chainsign_element(msg, msg_sign, m_username, get_public_key()));
    return tok;
}

std::string c_user::get_token_packet(const std::string &user_pubkey, bool keep_in_wallet) {

  try {
    c_token tok = process_token_tosend(user_pubkey,keep_in_wallet);
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

bool c_user::send_token_bymethod(c_user &user, bool keep_in_wallet) {

  try {
    c_token tok = process_token_tosend(user.get_public_key(),keep_in_wallet);

    if(user.recieve_token(tok)) { // push this coin to the target user
        std::cout << "Fail to recieve token: token lost" << std::endl;
        // place to handle bad token
        // maybe send invalid token to arbiters?
        return true;
    }
    return false;
  } catch(std::exception &ec) {
    std::cerr << ec.what() << std::endl;
  }
}

bool c_user::recieve_from_packet(std::string &packet) {

    c_token tok(packet);
    if(recieve_token(tok)) {
        std::cout << "Fail to recieve token: token lost" << std::endl;
        // place to handle bad token
        // maybe send invalid token to arbiters?
        return true;
    }
    return false;
}

bool c_user::recieve_token (c_token &token) {
    token.print(std::cout,1);
    std::cout << "Refreshing local tokens status before recieving new" << std::endl;
    tokens_refresh();
    token.print(std::cout,1);

    if(m_evidences.token_date(token) || m_evidences.mint_check(token)) {
        return true;
    }
	// check validity of the signatures chain
	std::string expected_sender; // publickey
	bool expected_sender_any=false; // do we expecected sender

	// [A->B]   [B->C]   [C->D]
    for (auto &current_signature : token.get_chainsign()) {
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
            std::cout << "token validate : BAD_EXPECTED_SENDER !!!" << std::endl;
            return true;
		}

		if (!ok_sign) {
			std::cout << "token validate : BAD_SIGN !!!" << std::endl;
            return true;
		}

		expected_sender = current_recipient_in_coin;
		expected_sender_any = true;

        std::cout << "CHAINSIGN_INFO" << std::endl;
        current_signature.print(std::cout);
	}

	std::cout << "token validate : OK" << std::endl;
	std::cout << "size of this token : " << token.get_size() << std::endl;

    if (m_mint.check_isEmited(token)) { // is this token emitted by me?
        for (auto &in : m_used_tokens) { // is this token used?
            if (in == token) {
                std::cout << "token validate : TOKEN_USED !!!" << std::endl;
                if(!m_evidences.find_token_cheater(token, in)) {
                    std::cout << "can't find cheater" << std::endl;
                } else {
                    return true;
                }
            }
        }
        m_used_tokens.emplace_back(std::move(token));
        return false;		// TODO should we replace token by new one?
    }
    m_wallet.move_token(std::move(token));
    return false;
}

void c_user::set_new_mint(std::string mintname, std::string pubkey, std::chrono::seconds exp_time) {

    m_mint = c_mint(mintname,pubkey,exp_time);
}

size_t c_user::clean_expired_tokens() {

    size_t size_before = m_used_tokens.size();
    size_t last_id = get_mint_last_expired_id();
    m_used_tokens.erase(
            std::remove_if(	m_used_tokens.begin(),
                            m_used_tokens.end(),
                            [&last_id] (const c_token &element) {
                                    if(element.get_id() < last_id){
                                        return true;
                                    }
                                    return false;
                            }),
            m_used_tokens.end());

    size_t expired_amount = size_before - m_used_tokens.size();
    return expired_amount;
}

size_t c_user::tokens_refresh() {
    size_t all_expired_tokens = 0;
    all_expired_tokens += m_mint.clean_expired_tokens();
    all_expired_tokens += m_wallet.clean_expired_tokens();
    all_expired_tokens += clean_expired_tokens();

    return all_expired_tokens;
}

void c_user::emit_tokens (size_t amount) {
	for(size_t i = 0; i < amount; ++i) {
        m_wallet.move_token(m_mint.emit_token());
    }
}

long c_user::get_mint_last_expired_id() const {
    return m_mint.get_last_expired_id();
}

void c_user::print_status(std::ostream &os) const {
    os << "|||<<>> " << m_username << " <<>>||| status info:" << std::endl;
    m_mint.print_mint_status(os);
    print_used_status(os);
    m_wallet.print_wallet_status(os,true);
}

void print_strBytes(const std::string& str) {
	for(size_t i = 0; i < str.length(); ++i) {
		std::cout << static_cast<int>(str[i]) << ":";
	}
	std::cout << std::endl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// coinwallet

void c_user::save_coinwallet(const std::string &filename) {
    m_wallet.save_to_file(filename);
}

void c_user::load_coinwallet(const std::string &filename) {
    m_wallet.load_from_file(filename);
}

//////////////////////////////////////////////////////////////////////////////////////////////////// bitwallet

bool c_user::check_bitwallet() {
    return m_bitwallet.is_set();
}

void c_user::set_bitwallet(const std::string &rpc_username, const std::string &rpc_password, const std::string &rpc_address, int rpc_port) {

    m_bitwallet.set_wallet(rpc_username, rpc_password, rpc_address, rpc_port);
}
double c_user::get_bitwallet_balance() {
    return get_bitwallet_balance();
}
