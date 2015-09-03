#include "c_mint.h"

long generate_pass() {

   return 12345;
}

c_mint::c_mint()
{

}

c_token c_mint::emit_token() {

    c_token token;
    m_emited_tokens[token.id] = generate_pass();

    return token;
}
