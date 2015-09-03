//
// Created by work on 03.09.15.
//

#include "c_wallet.hpp"

c_wallet::c_wallet (const string &name) : tokens_type(name) { }

c_wallet::c_wallet (string &&name) : tokens_type(name) { }

size_t c_wallet::amount () { return tokens.size(); }
