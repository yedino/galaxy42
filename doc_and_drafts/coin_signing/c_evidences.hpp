#ifndef C_EVIDENCES_H
#define C_EVIDENCES_H

#include "../../crypto_ops/crypto/c_encryption.hpp"
#include "c_token.hpp"
#include "coinsign_error.hpp"
/***
 * Class that provide evidences methods
 * for ensuring transactions security
 */

class c_evidences {
  public:
    c_evidences (c_ed25519 &ed);
    /// double spending cheater finding
    /// true means the cheater was found
    bool find_token_cheater (const c_token &, const c_token &) const;

    /// check if emiter is correct in case of stolen database
    /// true means stolen token
    bool mint_check (const c_token &) const;

    /// check if token is deprecated
    /// true means deprecated token
    bool token_date (const c_token &) const;

  private:
    c_ed25519 &m_edsigner;
};

#endif // C_EVIDENCES_H
