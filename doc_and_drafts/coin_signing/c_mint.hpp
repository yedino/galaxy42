#ifndef C_MINT_H
#define C_MINT_H

#include "c_token.hpp"
#include "../../crypto_ops/crypto/c_random_generator.hpp"
#include <map>
#include <string>

using std::string;

class c_mint {
  public:
	c_mint () = default;

	c_token emit_token ();

	bool check_isEmited (c_token &);
    void print_mint_status(std::ostream &os);
  private:
	std::map<c_token, long long> m_emited_tokens;
	c_random_generator<long long> random_generator;

	long long generate_password ();
};

#endif // C_MINT_H

