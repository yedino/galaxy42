#pragma once
#ifndef glue_lockedstring_trivialserialize
#define glue_lockedstring_trivialserialize

#include "trivialserialize.hpp"

#include <sodium.h>
#include "strings_utils.hpp"

void memlock(const trivialserialize::generator & gen) {
	sodiumpp::mlock( const_cast<std::string&>(gen.get_buffer()) ); // TODO(rob) const string memlock
}

void serialize(const sodiumpp::locked_string & obj, trivialserialize::generator & gen) {
	UNUSED(obj);
	UNUSED(gen);
	TODOCODE;
}

template<typename T>
int foo() { T a; return 42; }

template<typename T>
T deserialize( trivialserialize::parser & parser) { UNUSED(parser); TODOCODE; }

template<>
sodiumpp::locked_string deserialize<sodiumpp::locked_string>( trivialserialize::parser & parser) {
	//sodiumpp::locked_string str;
	UNUSED(parser);
	TODOCODE;
}

#endif


