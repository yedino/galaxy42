#include "c_user.hpp"

c_user::c_user (const std::string& username) :
    m_edkeys(crypto_ed25519::generate_key()),
    m_mint(username,get_public_key())	// mintname could by other than username
{
    m_username = username;
    if (m_username.find('|') != string::npos) {
        throw std::runtime_error("nickname is not allowed to contain '|' character");
    }
    m_reputation = 1;
}

c_user::c_user (string &&username) :
    m_edkeys(crypto_ed25519::generate_key()),
    m_mint(username,get_public_key())	// mintname could by other than username
{
    m_username.swap(username);
	if (m_username.find('|') != string::npos) {
        throw std::runtime_error("nickname is not allowed to contain '|' character");
	}
    m_reputation = 1;
}

c_user::c_user(c_user && user) :
                                 m_edkeys(std::move(user.m_edkeys)),
                                 m_mint(std::move(user.m_mint)),
                                 m_wallet(std::move(user.m_wallet)),
                                 m_seen_tokens(std::move(user.m_seen_tokens)),
                                 m_username(std::move(user.m_username)),
                                 m_reputation(std::move(user.m_reputation))
{ }

std::string c_user::get_username() const {
	return m_username;
}
void c_user::set_username(std::string username) {
    m_username = username;
}

ed_key c_user::get_public_key() const{
    return m_edkeys.public_key;
}

double c_user::get_rep() {
	return atan(m_reputation)*100*(2/M_PI);
}

void c_user::print_seen_status(std::ostream &os) const {
    os << "Amount of used tokens: " << m_seen_tokens.size() << std::endl;
    for(auto &tok : m_seen_tokens) {
        os << "Id: [" << tok.get_id() << "], Size: [" << tok.get_size() << "B: ]" << std::endl;
    }
}

c_token c_user::process_token_tosend(const ed_key &user_pubkey, bool keep_in_wallet) {
    std::lock_guard<std::mutex> lck (m_mtx);
    if(m_wallet.process_token()) {
        std::string msg = m_username + " can't send token -- transaction abort";
        throw std::logic_error(msg);
    }
    c_token tok = m_wallet.get_any_token(keep_in_wallet); // take any token for now [TODO]

    std::stringstream msg_stream;
    msg_stream << tok.get_id() << '|' << user_pubkey;
    std::string msg = msg_stream.str();
    ed_key msg_sign = crypto_ed25519::sign(msg,m_edkeys);

    tok.add_chain_element(c_chainsign_element(msg, msg_sign, m_username, get_public_key()));
    return tok;
}

std::string c_user::get_token_packet(int method, const ed_key &user_pubkey, bool keep_in_wallet) {

  try {
    c_token tok = process_token_tosend(user_pubkey,keep_in_wallet);
    m_mtx.lock();

    std::string packet = tok.to_packet(method);
    return packet;

  } catch(const std::logic_error &l_err) {
        std::cerr << l_err.what() << std::endl;
        std::string fail = "fail";
        return fail;
  }
}

bool c_user::send_token_bymethod(c_user &user, bool keep_in_wallet) {

  try {
    c_token tok = process_token_tosend(user.get_public_key(),keep_in_wallet);
    std::cout << m_username << ": sending [" << tok.get_emiter_name() << "] token with id [" << tok.get_id() << "] by method to [" << user.get_username() << "]" << std::endl;
    if(user.recieve_token(tok)) { // push this coin to the target user
        std::cout << "Fail to recieve token: token lost" << std::endl;
        // place to handle bad token
        // maybe send invalid token to arbiters?
        return true;
    }
  } catch(const std::logic_error &l_err) {
        std::cerr << l_err.what() << std::endl;
  }
    return false;
}

bool c_user::recieve_from_packet(std::string &packet) {

  try {
    c_token tok(packet, 2);	// 2 means json method (mayby TODO enum)
    if(recieve_token(tok)) {
        std::cout << "Fail to recieve token: token lost" << std::endl;
        // place to handle bad token
        // maybe send invalid token to arbiters?
        return true;
    }
    //unsigned version = boost::archive::BOOST_ARCHIVE_VERSION();
    //std::cout << "Deserialize token with boost::archive version : " << version << std::endl;

  } catch(const std::logic_error &l_err) {
        std::cerr << l_err.what() << std::endl;
        return true;
  } catch(const boost::archive::archive_exception &b_err) {
        std::cout << b_err.what() << std::endl;
        return true;
  }
    return false;
}

bool c_user::recieve_token (c_token &token) {
    //std::cout << "Refreshing local tokens status before recieving new" << std::endl;
    tokens_refresh();

    if(coinsign_evidences::token_date(token)) {
        throw coinsign_error(12,"DEPRECATED TOKEN - bad token date");
    }
    if(coinsign_evidences::mint_check(token)) {
        throw coinsign_error(13,"MINT CHECK FAIL - bad mint public key");
    }
	// check validity of the signatures chain
    ed_key expected_sender; // publickey
    bool expected_sender_any = false; // do we expecected sender

	// [A->B]   [B->C]   [C->D]
    for (auto &current_signature : token.get_chainsign()) {
        bool ok_sign = crypto_ed25519::verify_signature(current_signature.m_msg,
                                                        current_signature.m_msg_sign,
                                                        current_signature.m_signer_pubkey);
        if (!ok_sign) {
            std::cout << "token validate : BAD_SIGN !!!" << std::endl;
            throw coinsign_error(11,"TOKEN VALIDATE FAIL - bad sign");
        }

        ed_key current_sender_in_coin = current_signature.m_signer_pubkey;
        ed_key current_recipient_in_coin;

		std::string delimeter = "|";
		std::size_t found = current_signature.m_msg.find(delimeter)+1; // +1 to avoid delimeter
        if (found != std::string::npos) {
            std::stringstream in_hex;
            in_hex << current_signature.m_msg.substr(found);
            in_hex >> current_recipient_in_coin;
          //  current_recipient_in_coin << current_signature.m_msg.substr(found);
        }

		// [B->C] is the current sender B allowed to send,  check for error:
        if ((expected_sender_any) && current_sender_in_coin != expected_sender) {
            std::cout << "expected sender [" << expected_sender <<
                         "] vs in-coin sender [" << current_sender_in_coin << "]" << std::endl;
            throw coinsign_error(10,"TOKEN VALIDATE FAIL - bad expected sender");
		}

		expected_sender = current_recipient_in_coin;
		expected_sender_any = true;

        //std::cout << "CHAINSIGN_INFO" << std::endl;
        //current_signature.print(std::cout);
	}

    //std::cout << "token validate : OK" << std::endl;
    //std::cout << "size of this token : " << token.get_size() << std::endl;

    if(m_mint.check_is_emited(token)) { // is this token emitted by me?
        bool is_ok = m_mint.get_used_token(token);
        return is_ok;
    } else {
        bool seen = false;
        for (auto &in : m_seen_tokens) { // is this token used?
            if (in == token) {
                //std::cout << "TOKEN_SEEN - checking is it double spend" << std::endl;
                if(coinsign_evidences::simple_malignant_cheater(token, in, m_username)) {
                    std::cout << "USER: TOKEN_SEEN - found malignant cheater" << std::endl;
                    throw coinsign_error(15,"DOUBLE SPENDING - found cheater");
                }
                else if(coinsign_evidences::find_token_cheater(token, in, m_username)) {
                    std::cout << "USER: TOKEN_SEEN - found cheater" << std::endl;
                    throw coinsign_error(15,"DOUBLE SPENDING - found cheater");
                }
                // after this line cheater test does not make sense
                // replacing new seen version of token
                m_seen_tokens.remove(in);
                m_seen_tokens.push_back(token);
                seen = true;
                break;
            }
        }
        if(!seen) {
            m_seen_tokens.push_back(token);
        }
    }
    //std::cout << m_username << ": move token to wallet" << std::endl;
    m_wallet.move_token(std::move(token));
    return false;
}

void c_user::set_new_mint(std::string mintname, ed_key pubkey, std::chrono::seconds exp_time) {

    m_mint = c_mint(mintname,pubkey,exp_time);
}

size_t c_user::clean_expired_tokens() {

    size_t size_before = m_seen_tokens.size();
    m_seen_tokens.erase(
            std::remove_if(	m_seen_tokens.begin(),
                            m_seen_tokens.end(),
                            [] (const c_token &element) {
                                    if(element.get_expiration_date() < std::chrono::system_clock::now()) {
                                        std::cout << "User : remove seen deprecated token: "  << std::endl;
                                        element.print(std::cout);
                                        return true;
                                    }
                                    return false;
                            }),
            m_seen_tokens.end());

    size_t expired_amount = size_before - m_seen_tokens.size();
    return expired_amount;
}

size_t c_user::tokens_refresh() {
    size_t all_expired_tokens = 0;
    all_expired_tokens += m_mint.clean_expired_emited();
    all_expired_tokens += m_mint.clean_expired_used();
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

void c_user::print_status(std::ostream &os, bool verbouse) const {
    os << "||| " << m_username << " ||| status info:" << std::endl;
    m_wallet.print_wallet_status(os,verbouse);
    m_mint.print_mint_status(os);
    if(verbouse) {
        print_seen_status(os);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////// saving state

void c_user::save_user(std::string filename) const {
    if(filename == "default") {
        filename = m_username;
        filename += ".dat";
    }
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    std::cout << m_username << ": saving data to [" << filename << "]" << std::endl;
    oa << *this;
}

bool c_user::load_user(std::string filename) {
  try {
    if(filename == "default") {
        filename = m_username;
        filename += ".dat";
    }
    if(cs_utils::file_exsist(filename)) {
        std::ifstream ifs(filename);
        boost::archive::text_iarchive ia(ifs);
        std::cout << "Loading data from [" << filename << "]" << std::endl;
        ia >> *this;
    } else {
        std::cout << "Load "  << filename << " fail" << std::endl;
        throw std::runtime_error("file does not exist");
    }
  } catch (std::exception &ec) {
          std::cerr << "Exception opening/reading/closing file :" << ec.what() << std::endl;
          return false;
  }
    return true;
}

void c_user::save_coinwallet(const std::string &filename) const {
    m_wallet.save_to_file(filename);
}

void c_user::load_coinwallet(const std::string &filename) {
    m_wallet.load_from_file(filename);
}

void c_user::save_keys() const {
    //TODO
}

void c_user::load_keys() {
    //TODO
}

//////////////////////////////////////////////////////////////////////////////////////////////////// bitwallet

//bool c_user::check_bitwallet() {
//    return m_bitwallet.is_set();
//}

//void c_user::set_bitwallet(const std::string &rpc_username, const std::string &rpc_password, const std::string &rpc_address, int rpc_port) {

//    m_bitwallet.set_wallet(rpc_username, rpc_password, rpc_address, rpc_port);
//}
//double c_user::get_bitwallet_balance() {
//    return get_bitwallet_balance();
//}
