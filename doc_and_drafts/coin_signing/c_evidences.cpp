#include "c_evidences.hpp"

namespace form_print {

    int col_size = 15;

    void target_print(const std::string &lt, const std::string &rt) {
        int w1 = (2*col_size-lt.size())/2;
        int w2 = (2*col_size-rt.size())/2-lt.size()/2;

        if(lt.empty()) {
            std::cout 	<< std::setw(w1+w2+1) << '[' << rt << ']';
        }
        else if(rt.empty()) {
            std::cout 	<< std::setw(w1) << '[' << lt << ']';
        }
        else {
            std::cout 	<< std::setw(w1) << '[' << lt << ']'
                        << std::setw(w2) << '[' << rt << ']';
        }
        if(w1<1 || w2<1) {
            std::cout 	<< " *PRETTY FORMATTING FAIL - " << w1 << " : " << w2 << " - "
                        << " too long usernames or too small col_size - check it" << std::endl;
        }
        else {
            std::cout << std::endl;
        }
    }

    void arrow_print(bool left, bool right) {
        if(left && !right) {
            std::cout << std::setw(col_size) << '|' << std::endl;
            std::cout << std::setw(col_size) << 'V' << std::endl;
        }
        else if(!left && right) {
            std::cout << std::setw(2*col_size+1) << '|' << std::endl;
            std::cout << std::setw(2*col_size+1) << 'V' << std::endl;
        }
        else if(left && right) {
            std::cout << std::setw(col_size) << '|' << std::setw(col_size+1) << '|' << std::endl;
            std::cout << std::setw(col_size) << 'V' << std::setw(col_size+1) << 'V' << std::endl;
        }
    }

    void cheater_print(std::string &cheater) {
        std::string l1 = "*** !!! DOUBLE SPENDING detected !!! ***";
        std::string l2 = "*** !!! the CHEATER is: " + cheater + " !!! ***";
        std::cout << std::setw(col_size*1.5 + l1.size()/2) << l1 << std::endl;
        std::cout << std::setw(col_size*1.5 + l2.size()/2) << l2 << std::endl;
    }
}

namespace coinsign_evidences {

    bool find_token_cheater (const c_token &token_a, const c_token &token_b) {

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

        // [A->B]   [B->C]   [C->D] token_a
        // [A->B]   [B->C]   [C->X] token_b if he cheated at end

        bool is_dbspend = false;
        form_print::target_print(std::string("TOKEN A"), std::string("TOKEN B"));

        auto tok_a_chain = token_a.get_chainsign();
        auto tok_b_chain = token_b.get_chainsign();

        for (auto pos=0; pos<len_min; ++pos) {
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

            form_print::target_print(current_signature_a.m_signer, current_signature_b.m_signer);
            form_print::arrow_print(true, true);

            if (current_signature_a != current_signature_b && !is_dbspend) {
                form_print::cheater_print(current_signature_a.m_signer);
                form_print::arrow_print(true, true);
                is_dbspend = true;
            }
        }
        if(len_min == 2) {
                std::string cheater = tok_a_chain[1].m_signer;
                form_print::cheater_print(cheater);
                is_dbspend = true;
                if(!different_size) {
                    form_print::arrow_print(true,true);
                    form_print::target_print(tok_a_chain[0].m_signer,tok_b_chain[0].m_signer);
                }
                return true;
        }
        if(different_size == false) {
            form_print::target_print(tok_a_chain[0].m_signer,tok_b_chain[0].m_signer);
            if(is_dbspend) {
                return true;
            } else {
                return false;
            }
        } else if(bigger == 'a') {
            form_print::target_print("*!* COPIED *!*",tok_b_chain[0].m_signer);
            form_print::arrow_print(true,false);
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

                form_print::target_print(current_signature_a.m_signer,"");
                form_print::arrow_print(true,false);
            }
            form_print::target_print(tok_a_chain[0].m_signer,"");
            return true;
        } else if(bigger == 'b') {
            form_print::target_print(tok_a_chain[0].m_signer,"*!* COPIED *!*");
            form_print::arrow_print(false,true);
            for (auto pos=len_min; pos<len_max; ++pos) {
                auto &current_signature_b = tok_b_chain[pos];

                form_print::target_print("",current_signature_b.m_signer);
                form_print::arrow_print(false,true);
            }
            form_print::target_print("",tok_b_chain[0].m_signer);
            return true;
        } else {
            throw std::logic_error("find_the_cheater: unexpected error");
        }
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
        std::cout << "MINT_CHECK : ok" << std::endl;
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
