#ifndef C_EVIDENCES_H
#define C_EVIDENCES_H

#include "../../crypto_ops/crypto/crypto_ed25519.hpp"
#include "c_token.hpp"
#include "coinsign_error.hpp"
/***
 * Class that provide evidences methods
 * for ensuring transactions security
 */

namespace coinsign_evidences {

    /// double spending cheater finding
    /// true means the cheater was found
    bool find_token_cheater (const c_token &, const c_token &);

    /// double spending chaeter finding
    /// simple malignant test in case two tokens have the same history
    /// true means the cheater is last sender
    bool simple_malignant_cheater(const c_token &, const c_token &);

    /// check if emiter is correct in case of stolen database
    /// true means stolen token
    bool mint_check (const c_token &);

    /// check if token is deprecated
    /// true means deprecated token
    bool token_date (const c_token &);

}

#endif // C_EVIDENCES_H
