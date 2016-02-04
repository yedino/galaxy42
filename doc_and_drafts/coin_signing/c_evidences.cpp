#include "c_evidences.hpp"

c_evidences::c_evidences(c_ed25519 &ed) : m_edsigner(ed)
{ }

bool c_evidences::find_token_cheater (const c_token &token_a, const c_token &token_b) {

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
        throw std::runtime_error("find_the_cheater: unexpected error");
    }
}
