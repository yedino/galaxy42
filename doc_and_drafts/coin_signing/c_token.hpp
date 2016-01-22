#ifndef COIN_SIGNING_C_TOKEN_HPP
#define COIN_SIGNING_C_TOKEN_HPP
#include <cstdlib>
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>

class token_id_generator {
	static size_t id;
  public:
	static size_t generate_id ();
};

struct c_chainsign_element {
	c_chainsign_element (const std::string, const std::string, const std::string, const std::string);
    c_chainsign_element (const std::string &);	// deserialize chainelement from packet

    std::string m_msg;
    std::string m_msg_sign;
    std::string m_signer;
    std::string m_signer_pubkey;
};

struct c_token {
    size_t id;
	std::vector<c_chainsign_element> m_chainsign;

	c_token (long long);
    c_token (std::string);		///< deserialize token from recived packet

    std::string to_packet();	///< serialize token

    long long get_size();
    bool check_ps (long long);

  private:
	long long m_password;
};

bool operator != (const c_chainsign_element &,const c_chainsign_element &);

bool operator == (const c_token &, const c_token &);

bool operator < (const c_token &, const c_token &);

#endif //COIN_SIGNING_C_TOKEN_HPP


