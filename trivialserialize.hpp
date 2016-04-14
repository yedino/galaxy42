#ifndef include_trivialserialize_hppaaa
#define include_trivialserialize_hppaaa

#include "libs1.hpp"

using namespace std;

namespace trivialserialize {

void test_trivialserialize();

// ---


class format_error : public std::exception { public:	const char * what() const noexcept override; };

class format_error_read : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_badformat : public format_error { public:	const char * what() const noexcept override; };


class format_error_write : public format_error { public:	const char * what() const noexcept override; };

class format_error_write_too_long : public format_error_write { public:	const char * what() const noexcept override; };

class format_error_write_value_too_big : public format_error { public:	const char * what() const noexcept override; };


/***
@brief Generates the serialized data string, from input data like bytes and strings of various types.
@note About string types, following are possible:
- conststr: strings of constant length of N characters (during read, and during write, the caller knows the size is exactyl N)
- octetsvarstr: We support strings that length is variable, but it is known (during read, and write, the caller tells us) to which string-length the string belongs:
-- is it a string with length <= 2^(8*S) -1 for S=1 (using 1 octet to encode own width)
-- is it a string with length <= 2^(8*S) -1 for S=2
-- is it a string with length <= 2^(8*S) -1 for S=3
-- is it a string with length <= 2^(8*S) -1 for S=4
-- If the program tells the string has S=3 but actuall data is always shorter then e.g. 256, so would fit in S=1, then this is fine of course, though.
always there is wasted space (e.g. of 1 byte) in the serialized format.
- varstring: we COULD have one that uses 4 bytes to encode length when it is long, but uses just 1 byte when it is e.g. 250; bitcoin uses such mechanism. Not implemented (yet?).
- cstring: we COULD have null-delimited string. Not implemented (yet?).
*/
class generator {
	protected:
		std::string m_str; ///< the generated data so far

		// powers of two for different number of bytes (8-bit - octets):
		constexpr static size_t bytesize0 = 1;
		constexpr static size_t bytesize1 = 1 << (8*1); // and size of 1 byte, so usable for module
		constexpr static size_t bytesize2 = 1 << (8*2); // size for 2 octet word
		constexpr static size_t bytesize3 = 1 << (8*3); // 3 byte
		constexpr static size_t bytesize4minus1 = (1LL << (8*4)) -1; // typical 4 octet bigger word, but -1 (so it fits in size_t), we need other comparsion when using it (<=).

	public:
		generator(size_t suggested_size);

		void push_byte_u(unsigned char c);
		void push_byte_s(signed char c);

		template <int S, typename T> void push_integer_u(T value); ///< Saves some unsigned integer-type S, into field that is S octets wide.
		template <int S, typename T> void push_integer_s(T value); ///< Saves some   signed integer-type S, into field that is S octets wide.

		/***
		@brief writes conststr (see class notes).
		@note Supported data size is 0 to 2^32 -1.
		@note Serialized size will be <= the data size.
		@param size - the defined exact size of data. We require (and assert) that it == data.size(). It is used ONLY for asserting.
		@param data - the string of binary data
		*/
		void push_bytes_n(size_t size, const std::string & data);

		/***
		@brief writes octetsvarstr (see class notes).
		@note Supported data size is 0 to 2^32 -1 in general, and for given S it is 2^(8*S)-1
		@note Serialized size will be <= the data size + S
		@param S - the numbers of octets needed to express the maximum possible size of string. String can have at most (2^(8*S)-1) characters.
		@param data - the string of binary data.
		@param max_size_for_assert - the expected maximum size of data. It can be lower, e.g. S=1 but we want to assert string is <=100 characters. Is asserted.
		*/
		template <int S> void push_bytes_sizeoctets(const std::string & data, size_t max_size_for_assert);
		template <int S> void push_bytes_sizeoctets(const std::string & data); //< same as push_bytes_sizeoctets with max_size_for_assert, except that argument is not used (and this additional assert is not done)

		const std::string & str() const; ///< get the generated string. This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!

	protected:
		void push_bytes_octets_and_size(unsigned char octets, size_t max_size, const std::string & data); ///< give number of octets of actuall-data-size, give the max_size that is just asserted, and the data
};


/***
@brief For prasing serialized data. It must ncelly THROW in case if input is in format other then expected, e.g. trying to read more data then possible.
It absolutelly must at least assert all possible internal errors resulting from input that is logically invalid in any way (though of course
it must be technically valid, e.g. not nullptr string).
And this asserts must be always-on, because this is for parsing unsafe external data. TODO(r) abort() that can not be disabled
*/
class parser {
	public:
		struct tag_caller_must_keep_this_string_valid {} ;
		struct tag_caller_must_keep_this_buffer_valid {} ;

		// TODO(r): pick a guideline and use existing view idiom like that:
		const char * m_data_begin; ///< the begining of string
		const char * m_data_now; ///< current position (must be inside string, must be in range begin..end)
		const char * m_data_end;

		parser( tag_caller_must_keep_this_string_valid x , const std::string & data_str);
		parser( tag_caller_must_keep_this_buffer_valid x , const char * buf , size_t size);

		unsigned char pop_byte_u();
		signed char pop_byte_s();

		// TODO(r) deduce type T fitting from S octets:
		template <int S, typename T> T pop_integer_u(); ///< Reads some unsigned integer-type S, into field that is S octets wide, as value of type T.
		template <int S, typename T> T pop_integer_s(); ///< Reads some   signed integer-type S, into field that is S octets wide, as value of type T.

		std::string pop_bytes_n(size_t size); //< Read and return binary string of exactly N characters always, that was saved using push_bytes_n()

		template <int S> std::string pop_bytes_sizeoctets();
};


} // namespace


#endif

