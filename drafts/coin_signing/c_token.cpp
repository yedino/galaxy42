#include "c_token.hpp"

size_t token_id_generator::id = 0;

c_token::c_token () : id(token_id_generator::generate_id()) { }

size_t token_id_generator::generate_id () { return id++; }

bool operator== (const c_token &lhs, const c_token &rhs) { return lhs.id == rhs.id; }

bool operator< (const c_token &lhs, const c_token &rhs) { return lhs.id < rhs.id; }