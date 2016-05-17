#pragma once
#ifndef glue_lockedstring_trivialserialize
#define glue_lockedstring_trivialserialize

#include "trivialserialize.hpp"

#include <sodiumpp/sodiumpp.h>
#include "libs0.hpp"
#include "strings_utils.hpp"

namespace trivialserialize {

void memlock(const generator & gen);

template <> void serialize<sodiumpp::locked_string>(const sodiumpp::locked_string & data, generator & gen);

template <> sodiumpp::locked_string deserialize<sodiumpp::locked_string>(parser & parser);

}	// namespace trivialserialize
#endif


