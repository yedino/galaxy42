#pragma once
#ifndef glue_lockedstring_trivialserialize
#define glue_lockedstring_trivialserialize

#include "trivialserialize.hpp"

#include <sodiumpp/sodiumpp.h>
#include "libs0.hpp"
#include "strings_utils.hpp"

void memlock(const trivialserialize::generator & gen) {
	sodiumpp::mlock( const_cast<std::string&>(gen.get_buffer()) ); // TODO(rob) const string memlock
}

template <> void trivialserialize::serialize(const sodiumpp::locked_string & data, trivialserialize::generator & gen);
template <> sodiumpp::locked_string trivialserialize::deserialize<sodiumpp::locked_string>(trivialserialize::parser & parser);

#endif


