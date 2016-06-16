#include "glue_lockedstring_trivialserialize.hpp"

void trivialserialize::memlock(const trivialserialize::generator &gen) {
	sodiumpp::mlock( const_cast<std::string&>(gen.get_buffer()) ); // TODO(rob) const string memlock
}

template <> void trivialserialize::obj_serialize<sodiumpp::locked_string>(
	const sodiumpp::locked_string & data, trivialserialize::generator & gen)
{
	gen.push_integer_uvarint(data.size()); // save the length varint
	gen.push_bytes_n(data.size(), data.get_string()); // save the data
}

template <> sodiumpp::locked_string trivialserialize::obj_deserialize<sodiumpp::locked_string>(
trivialserialize::parser & parser)
{
	auto size = parser.pop_integer_uvarint(); // load the size
	sodiumpp::locked_string ret(size); // prepare mem
	parser.pop_bytes_n_into_buff(size, ret.buffer_writable());
	return ret;
}
