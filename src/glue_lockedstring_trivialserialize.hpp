// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef glue_lockedstring_trivialserialize
#define glue_lockedstring_trivialserialize

#include "trivialserialize.hpp"

#include <sodiumpp/sodiumpp.h>
#include "libs0.hpp"
#include "strings_utils.hpp"

namespace trivialserialize {


void memlock(const trivialserialize::generator &gen); ///< mem lock entire generator

// provide the obj_serialize API for trivialserialize

template <> void obj_serialize<sodiumpp::locked_string>(const sodiumpp::locked_string & data, generator & gen);

template <> sodiumpp::locked_string obj_deserialize<sodiumpp::locked_string>(parser & parser);

}	// namespace trivialserialize
#endif


