#ifndef include_trivialserialize_hppaaa
#define include_trivialserialize_hppaaa

#include "libs1.hpp"

// yeap defines are not so nice but also remove some problems with global constants re init ordering for example
// TODO constexpr?
#define SANE_MAX_SIZE_FOR_STRING 50000000

using namespace std;

namespace trivialserialize {

void test_trivialserialize();

// ---


class format_error : public std::exception { public:	const char * what() const noexcept override; };

class format_error_read : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_badformat : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_delimiter : public format_error { public:	const char * what() const noexcept override; };


class format_error_write : public format_error { public:	const char * what() const noexcept override; };

class format_error_write_too_long : public format_error_write { public:	const char * what() const noexcept override; };

class format_error_write_value_too_big : public format_error { public:	const char * what() const noexcept override; };


/***
@brief Generates the serialized data string, from input data like bytes and strings of various types, there are lower and higher
level APIs in groups of functions.
@note About string types, following are possible:
- conststr: strings of constant length of N characters (during read, and during write, the caller knows the size is exactyl N)
- octetsvarstr: We support strings that length is variable, but it is known (during read, and write, the caller tells us) to which string-length the string belongs:
-- is it a string with length <= 2^(8*S) -1 for S=1 (using 1 octet to encode own width)
-- is it a string with length <= 2^(8*S) -1 for S=2
-- is it a string with length <= 2^(8*S) -1 for S=3
-- is it a string with length <= 2^(8*S) -1 for S=4
-- If the program tells the string has S=3 but actuall data is always shorter then e.g. 256, so would fit in S=1, then this is fine of course, though.
always there is wasted space (e.g. of 1 byte) in the serialized format.
- varstring: see push_varstring() - can save string of any length, and the width is stored in dynamic format (but
for tiny stings especially of widths <=255 other options are slghtly better)
- cstring: we COULD have null-delimited string. This option probably will not be implemented (as the string would need to be
escaped or at least asserted on save)
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

		/// === static interface - you need to specify exact (or maximum) data size youself ===

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

		/// === dynamic interface - easy handling data of any runtime size without thinking about it yourself ===

		void push_integer_uvarint(uint64_t val); ///< Encode unsigned int dynamically on 1,3,5,9 octets like Bitcoin's CompactSize

		void push_varstring(const std::string &data); ///< Encode entire string of any length (but < max uint64) in dynamic format.
		
		/// === high level interface - handling data structures ===

		void push_vector_string(const vector<string> & data); ///< Save vector<string>

		/// === export the result ===

		const std::string & str() const; ///< get the generated string. This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!

	protected:
		void push_bytes_octets_and_size(unsigned char octets, size_t max_size, const std::string & data); ///< give number of octets of actuall-data-size, give the max_size that is just asserted, and the data
};

template <int S, typename T> void generator::push_integer_u(T value) {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	if (S==8) { // make this variant to even COMPILE only for S==8 and the other one not compile to fix unneeded warning
		//cerr<<S<<endl;
		if ( value >= 0xFFFFFFFFFFFFFFFF ) throw format_error_write_value_too_big();
	} else {
		//cerr<<S<<endl;
		if ( value >= ( (1ULL<<(8*S))  -1) ) throw format_error_write_value_too_big();
	}

	// TODO use proper type, depending on S
	uint64_t divider = 1LLU << ((S-1)*8); // TODO(r) style: LLU vs uint64_t
	for (auto i=0; i<S; ++i) {
		//cerr << "Serializing " << value << " into S="<<S<<" octets. divider="<<divider<<"..." << flush;
		auto this_byte = (value / divider) % 256;
		//cerr << " this_byte=" << this_byte << endl;
		assert( (this_byte>=0) && (this_byte<=255) ); // TODO(r) remove assert later, when unit tests/code review
		push_byte_u( this_byte );
		divider = divider >> 8; // move to next 8 bits
	}
}

template <int S, typename T> void generator::push_integer_s(T value) {
	throw std::runtime_error("Not implemented yet");
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data, size_t max_size_for_assert) {
	assert( data.size() <= max_size_for_assert);
	push_bytes_sizeoctets<S>(data);
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data) {
	static_assert(( (S>=1) && (S<=4) ) , "Unsupported number of octets that will express actuall-size of the data. Try using 1,2,3 or 4.");
	const auto size = data.size();
	push_integer_u<S>(size);
	m_str += data; // write the actuall data
}




/***
@brief For prasing serialized data. It must ncelly THROW in case if input is in format other then expected, e.g. trying to read more data then possible.
It absolutelly must at least assert all possible internal errors resulting from input that is logically invalid in any way (though of course
it must be technically valid, e.g. not nullptr string).
And this asserts must be always-on, because this is for parsing unsafe external data. TODO(r) abort() that can not be disabled
*/
class parser {
	private:
		// TODO(r): pick a guideline and use existing view idiom like that:
		const char * const  m_data_begin; ///< the begining of string
		const char * m_data_now; ///< current position (must be inside string, must be in range begin..end)
		const char * const m_data_end;

	public:
		struct tag_caller_must_keep_this_string_valid {} ;
		struct tag_caller_must_keep_this_buffer_valid {} ;

		parser( tag_caller_must_keep_this_string_valid x , const std::string & data_str);
		parser( tag_caller_must_keep_this_buffer_valid x , const char * buf , size_t size);

		/// === static interface - you need to specify exact (or maximum) data size youself ===

		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		unsigned char pop_byte_u();
		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		signed char pop_byte_s();
		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		void pop_byte_skip(char c); // read expected character (e.g. a delimiter)

		// TODO(r) deduce type T fitting from S octets:
		template <int S, typename T> T pop_integer_u(); ///< Reads some unsigned integer-type S, into field that is S octets wide, as value of type T.
		template <int S, typename T> T pop_integer_s(); ///< Reads some   signed integer-type S, into field that is S octets wide, as value of type T.

		std::string pop_bytes_n(size_t size); //< Read and return binary string of exactly N characters always, 
		//< that was saved using push_bytes_n(). N can be 0.

		template <int S> std::string pop_bytes_sizeoctets();

		/// === dynamic interface - easy handling data of any runtime size without thinking about it yourself ===

		uint64_t pop_integer_uvarint(); ///< Decode unsigned int of 1,3,5,9 octets saved by push_integer_uvarint
		std::string pop_varstring(); ///< Decode string of any length saved by push_varstring()

		/// === high level interface - handling data structures ===

		vector<string> pop_vector_string(); ///< Decode a vector<string> object saved with push_vector_string

};


template <int S, typename T> T parser::pop_integer_u() {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	// TODO test if type T is not overflowing

	T value=0;
	uint64_t multiplier = 1LLU << ((S-1)*8); // TODO(r) style: LLU vs uint64_t
	for (auto i=0; i<S; ++i) {
		unsigned int this_byte = pop_byte_u();
		// cerr << "Reading for S="<<S<<" this_byte = " << this_byte << " mul=" << multiplier << endl;
		value += this_byte * multiplier;
		multiplier = multiplier >> 8; // move to next 8 bits
	}
	return value;
}

template <int S, typename T> T parser::pop_integer_s() {
	throw std::runtime_error("Not implemented yet");
}


template <int S> std::string parser::pop_bytes_sizeoctets() {
	size_t size = pop_integer_u<S,size_t>();
	// here we have the size, and we moved pass the serialized bytes of size, string is next:
	return pop_bytes_n( size );
}

} // namespace


#endif

