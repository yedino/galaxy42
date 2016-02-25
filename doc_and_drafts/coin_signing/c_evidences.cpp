#include "c_evidences.hpp"

namespace form_print {

    int col_size = 15;

    std::string target_print(const std::string &lt, const std::string &rt) {
        std::ostringstream out;
        int w1 = (2*col_size-lt.size())/2;
        int w2 = (2*col_size-rt.size())/2-lt.size()/2;

        if(lt.empty()) {
            out 	<< std::setw(w1+w2+1) << std::setfill(' ') << '[' << rt << ']';
        }
        else if(rt.empty()) {
            out 	<< std::setw(w1) << std::setfill(' ') << '[' << lt << ']';
        }
        else {
            out 	<< std::setw(w1) << std::setfill(' ') << '[' << lt << ']'
                        << std::setw(w2) << std::setfill(' ') << '[' << rt << ']';
        }
        if(w1<1 || w2<1) {
            out 	<< " *PRETTY FORMATTING FAIL - " << w1 << " : " << w2 << " - "
                        << " too long usernames or too small col_size - check it" << std::endl;
        }
        else {
            out << std::endl;
        }
        return out.str();
    }

    std::string arrow_print(bool left, bool right) {
        std::ostringstream out;
        if(left && !right) {
            out << std::setw(col_size) << std::setfill(' ') << '|' << std::endl;
            out << std::setw(col_size) << std::setfill(' ') << 'V' << std::endl;
        }
        else if(!left && right) {
            out << std::setw(2*col_size+1) << std::setfill(' ') << '|' << std::endl;
            out << std::setw(2*col_size+1) << std::setfill(' ') << 'V' << std::endl;
        }
        else if(left && right) {
            out << std::setw(col_size) << std::setfill(' ') << '|' << std::setw(col_size+1) << '|' << std::endl;
            out << std::setw(col_size) << std::setfill(' ') << 'V' << std::setw(col_size+1) << 'V' << std::endl;
        }
        return out.str();
    }

    std::string cheater_print(std::string &cheater) {
        std::ostringstream out;
        std::string l1 = "*** !!! DOUBLE SPENDING detected !!! ***";
        std::string l2 = "*** !!! the CHEATER is: " + cheater + " !!! ***";
        out << std::setw(col_size*1.5 + l1.size()/2) << std::setfill(' ') << l1 << std::endl;
        out << std::setw(col_size*1.5 + l2.size()/2) << std::setfill(' ') << l2 << std::endl;
        return out.str();
    }
}

namespace coinsign_evidences {
//receiver
    bool find_token_cheater (const c_token &token_a, const c_token &token_b, const std::string &receiver) {

        std::ostringstream diagram;

        size_t tok_a_chain_size = token_a.get_chainsign_size();
        size_t tok_b_chain_size = token_b.get_chainsign_size();

        auto len_max = std::max(tok_a_chain_size, tok_b_chain_size);
        auto len_min = std::min(tok_a_chain_size, tok_b_chain_size);
        bool different_size = (len_max != len_min);
        char bigger = '\0';
        if(different_size) {
            if(tok_a_chain_size == len_max) {
                bigger = 'a';
            } else {
                bigger = 'b';
            }
        }

        bool is_dbspend = false;
        diagram << form_print::target_print(std::string("TOKEN A"), std::string("TOKEN B"));

        auto tok_a_chain = token_a.get_chainsign();
        auto tok_b_chain = token_b.get_chainsign();

        for (size_t pos = 0; pos<len_min; ++pos) {
            auto &current_signature_a = tok_a_chain[pos];
            auto &current_signature_b = tok_b_chain[pos];

            // we need to verify only new token A
            bool ok_sign_a = crypto_ed25519::verify_signature(current_signature_a.m_msg,
                                                              current_signature_a.m_msg_sign,
                                                              current_signature_a.m_signer_pubkey);
            if (!ok_sign_a) {
                std::cout << "token validate : BAD_SIGN !!!" << std::endl;
                throw coinsign_error(11,"TOKEN VALIDATE FAIL - bad sign");
            }

            diagram << form_print::target_print(current_signature_a.m_signer, current_signature_b.m_signer);
            diagram << form_print::arrow_print(true, true);

            if (current_signature_a != current_signature_b && !is_dbspend) {
                diagram << form_print::cheater_print(current_signature_a.m_signer);
                diagram << form_print::arrow_print(true, true);
                is_dbspend = true;
            }
        }
        if(different_size == false) {
            diagram << form_print::target_print(receiver,receiver);
            if(is_dbspend) {
                std::cout << diagram.str();
            }
            return is_dbspend;
        } else if(bigger == 'a') {
            diagram << form_print::target_print("*!* COPIED *!*",receiver);
            diagram << form_print::arrow_print(true,false);
            for (auto pos=len_min; pos<len_max; ++pos) {
                auto &current_signature_a = tok_a_chain[pos];

            // we need to verify only new token A
                bool ok_sign_a = crypto_ed25519::verify_signature(current_signature_a.m_msg,
                                                                  current_signature_a.m_msg_sign,
                                                                  current_signature_a.m_signer_pubkey);
                if (!ok_sign_a) {
                    std::cout << "token validate : BAD_SIGN !!!" << std::endl;
                    throw coinsign_error(11,"TOKEN VALIDATE FAIL - bad sign");
                }

                diagram << form_print::target_print(current_signature_a.m_signer,"");
                diagram << form_print::arrow_print(true,false);
            }
            diagram << form_print::target_print(receiver,"");

            if(is_dbspend) {
                std::cout << diagram.str();
            }
            return is_dbspend;
        } else if(bigger == 'b') {
            diagram << form_print::target_print(receiver,"*!* COPIED *!*");
            diagram << form_print::arrow_print(false,true);
            for (auto pos=len_min; pos<len_max; ++pos) {
                auto &current_signature_b = tok_b_chain[pos];

                diagram << form_print::target_print("",current_signature_b.m_signer);
                diagram << form_print::arrow_print(false,true);
            }
            diagram << form_print::target_print("",receiver);

            if(is_dbspend) {
                std::cout << diagram.str();
            }
            return is_dbspend;
        } else {
            throw std::logic_error("find_the_cheater: unexpected error");
        }
    }

    bool simple_malignant_cheater(const c_token &token_a, const c_token &token_b, const std::string &receiver) {

        std::ostringstream diagram;

        auto chain_a = token_a.get_chainsign();
        auto chain_b = token_b.get_chainsign();
        size_t size_a = chain_a.size();
        if(size_a == chain_b.size()) {
            for(size_t i = 0; i < size_a; ++i) {
                if(chain_a.at(i) != chain_b.at(i)) {
                    return false;
                }
            }
            // printing cheater diagram
            diagram << form_print::target_print(std::string("TOKEN A"), std::string("TOKEN B"));
            int i = 0;
            for(auto chain_el_a : chain_a) {
                auto chain_el_b = chain_b.at(i);		// at is ok, after check size
                diagram << form_print::target_print(chain_el_a.m_signer, chain_el_b.m_signer);
                diagram << form_print::arrow_print(true, true);
                ++i;
            }
            std::string cheater = chain_a.at(size_a-1).m_signer;
            diagram << form_print::cheater_print(cheater);
            diagram << form_print::arrow_print(true,true);
            diagram << form_print::target_print(receiver,receiver);
            std::cout << diagram.str();
            return true;
        }
        return false;
    }

    bool mint_check(const c_token &tok) {

  try {
    //    std::string expecting_mintname = tok.get_emiter_name();
        ed_key expecting_mint_pubkey = tok.get_emiter_pubkey();

    //    std::string inchain_username = tok.get_chainsign().at(0).m_signer;
        ed_key inchain_mint_pubkey = tok.get_chainsign().at(0).m_signer_pubkey;

        if(!(expecting_mint_pubkey == inchain_mint_pubkey)) {
            std::cout << "MINT_CHECK FAIL : bad mint pubkey" << std::endl;
            return true;
        }
    //    else if(expecting_mintname != inchain_mintname) {			// TODO inchain username that own mine and mintname could be different
    //        std::cout << "MINT_CHECK FAIL : bad mint name" << std::endl;
    //        return true;
    //    }
  } catch (std::out_of_range &ec) {
        std::cout << ec.what() << std::endl;
        std::cout << "You can't check mint with never used token" << std::endl;
        return false;
  }
        //std::cout << "MINT_CHECK : ok" << std::endl;
        return false;
}

    bool token_date(const c_token &tok){

        if(tok.get_expiration_date() < std::chrono::system_clock::now()) {
            std::cout << "token validate : BAD_TOKEN_DATE !!!" << std::endl;
            std::cout << "Deprecated token : "; tok.print(std::cout,true);
            return true;
        }
        return false;
    }

}
