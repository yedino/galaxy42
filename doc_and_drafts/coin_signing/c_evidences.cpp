#include "c_evidences.hpp"

c_evidences::c_evidences(c_ed25519 &ed) : m_edsigner(ed)
{ }

bool c_evidences::find_token_cheater (const c_token &token_a, const c_token &token_b) const {

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
    std::cout << "[TOKEN A]" << "\t[TOKEN B]\n";

    auto tok_a_chain = token_a.get_chainsign();
    auto tok_b_chain = token_b.get_chainsign();

    for (auto pos=0; pos<len_min; ++pos) {
        auto &current_signature_a = tok_a_chain[pos];
        auto &current_signature_b = tok_b_chain[pos];

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
        std::cout << "[" << tok_a_chain[0].m_signer << "]\t["
                  << tok_b_chain[0].m_signer << "]" << std::endl;
        return true;
    } else if(bigger == 'a') {
        std::cout << "   V\t" << tok_b_chain[0].m_signer << "]" << std::endl;
        for (auto pos=len_min; pos<len_max; ++pos) {
            auto &current_signature_a = tok_a_chain[pos];


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
        std::cout << "[" << tok_a_chain[0].m_signer << "]\t"
                  << "\t" << std::endl;
        return true;
    } else if(bigger == 'b') {
        std::cout << "[" << tok_a_chain[0].m_signer << "]\t"
                  << "   V" <<  std::endl;
        for (auto pos=len_min; pos<len_max; ++pos) {
            auto &current_signature_b = tok_b_chain[pos];

            std::cout << "\t[" << current_signature_b.m_signer << "]\n"
                      << "   \t   |\n    \t   V" << std::endl;
        }
        std::cout << "\t[" << tok_b_chain[0].m_signer << "]" << std::endl;
        return true;
    } else {
        throw std::runtime_error("find_the_cheater: unexpected error");
    }
}

bool c_evidences::mint_check(const c_token &tok) const {

  try {
//    std::string expecting_mintname = tok.get_emiter_name();
    std::string expecting_mint_pubkey = tok.get_emiter_pubkey();

//    std::string inchain_username = tok.get_chainsign().at(0).m_signer;
    std::string inchain_mint_pubkey = tok.get_chainsign().at(0).m_signer_pubkey;

    if(expecting_mint_pubkey != inchain_mint_pubkey) {
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

bool c_evidences::token_date(const c_token &tok) const {

    if(tok.get_expiration_date() < std::chrono::system_clock::now()) {
        std::cout << "token validate : BAD_TOKEN_DATE !!!" << std::endl;
        std::cout << "Deprecated token : "; tok.print(std::cout,true);
        return true;
    }
    return false;
}
