#include "glue_lockedstring_trivialserialize.hpp"

void trivialserialize::memlock(const trivialserialize::generator &gen) {
	sodiumpp::mlock( const_cast<std::string&>(gen.get_buffer()) ); // TODO(rob) const string memlock
}

template <> void trivialserialize::serialize<sodiumpp::locked_string>(const sodiumpp::locked_string & data, trivialserialize::generator & gen) {
	UNUSED(data); UNUSED(gen);
}

template <> sodiumpp::locked_string trivialserialize::deserialize<sodiumpp::locked_string>(trivialserialize::parser & parser) {
	UNUSED(parser);
	sodiumpp::locked_string ret;
	return ret;
}
