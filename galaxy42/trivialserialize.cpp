

#include "trivialserialize.hpp"

#include "libs1.hpp"

using namespace std;

namespace trivialserialize {

/***

This code contains at least 1 intentional logical bug for excercise;

(In addition it probably contains other bugs too, it's all development code)

*/

class format_error : public std::exception { public:	const char * what() const noexcept override; };
const char * format_error::what() const noexcept { return "format-error in trivialserialize"; }

class format_error_read : public format_error { public:	const char * what() const noexcept override; };
const char * format_error_read::what() const noexcept { return "format-error in trivialserialize while reading, input data is invalid"; }

class format_error_read_badformat : public format_error { public:	const char * what() const noexcept override; };
const char * format_error_read_badformat::what() const noexcept { return "format-error in trivialserialize while reading, but it seems the format definition in the program is wrong"; }


class format_error_write : public format_error { public:	const char * what() const noexcept override; };
const char * format_error_write::what() const noexcept { return "format-error in trivialserialize while writting, the given data can not be serialized"; }

class format_error_write_too_long : public format_error_write { public:	const char * what() const noexcept override; };
const char * format_error_write_too_long::what() const noexcept { return "format-error in trivialserialize while writting, the given data can not be serialized - because data is too long (e.g. binary string)"; }

class format_error_write_value_too_big : public format_error { public:	const char * what() const noexcept override; };
const char * format_error_write_value_too_big::what() const noexcept { return "format-error in trivialserialize while writing - value was too big over limit"; }

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

generator::generator(size_t suggested_size)
	: m_str()
{
	m_str.reserve( suggested_size );
}

void generator::push_byte_u(unsigned char c) { 	m_str += c; }
void generator::push_byte_s(signed char c) {	m_str += c; }

template <int S, typename T> void generator::push_integer_u(T value) {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	if (S==8) { // make this variant to even COMPILE only for S==8 and the other one not compile to fix unneeded warning
		cerr<<S<<endl;
		if ( value >= 0xFFFFFFFFFFFFFFFF ) throw format_error_write_value_too_big();
	} else {
		cerr<<S<<endl;
		if ( value >= ( (1ULL<<(8*S))  -1) ) throw format_error_write_value_too_big();
	}

	// TODO use proper type, depending on S
	uint64_t divider = 1LLU << ((S-1)*8); // TODO(r) style: LLU vs uint64_t
	for (auto i=0; i<S; ++i) {
		cerr << "Serializing " << value << " into S="<<S<<" octets. divider="<<divider<<"..." << flush;
		auto this_byte = (value / divider) % 256;
		cerr << " this_byte=" << this_byte << endl;
		assert( (this_byte>=0) && (this_byte<=255) ); // TODO(r) remove assert later, when unit tests/code review
		push_byte_u( this_byte );
		divider = divider >> 8; // move to next 8 bits
	}
}

template <int S, typename T> void generator::push_integer_s(T value) {
	throw std::runtime_error("Not implemented yet");
}

void generator::push_bytes_n(size_t size, const std::string & data) {
	assert(size == data.size()); // is the size of data the same as size that we think should go here
	m_str += data;
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data, size_t max_size_for_assert) {
	assert( data.size() <= max_size_for_assert);
	push_bytes_sizeoctets<S>(data);
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data) {
	static_assert(( (S>=1) && (S<=4) ) , "Unsupported number of octets that will express actuall-size of the data. Try using 1,2,3 or 4.");

	const auto size = data.size();

	// write the actuall-data-size number:
	switch (S) {
		case 1:
			assert(size < bytesize1); // up to 255 char long sting
			push_byte_u(size); // the size expressed as one byte 0..255
		break;
		case 2:
			assert(size < bytesize2); // up to 2^(2*8) - 1 char long string
			push_byte_u((size / bytesize0) % bytesize1); // high byte
			push_byte_u((size / bytesize1)            ); // low byte; no need to modulo because division guarantees it will be in range
		break;
		case 3:
			assert(size < bytesize3); // up to 2^(3*8) - 1 char long string
			push_byte_u((size / bytesize0) % bytesize1); // highest byte
			push_byte_u((size / bytesize1) % bytesize1); //
			push_byte_u((size / bytesize2)            ); // lowest byte, no need to modulo [as above]
		break;
		case 4:
			assert(size <= bytesize4minus1); // up to 2^(4*8) - 1 char long string
			push_byte_u((size / bytesize0) % bytesize1); // highest byte
			push_byte_u((size / bytesize1) % bytesize1); //
			push_byte_u((size / bytesize2) % bytesize1); //
			push_byte_u((size / bytesize3)            ); // lowest byte, no need to modulo [as above]
		break;
		default: throw format_error_write_too_long(); // not supported
	}

	m_str += data; // write the actuall data
}


const std::string & generator::str() const { return m_str; }


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
		template <int S> std::string pop_bytes_sizeoctets() { return std::string(); }  // TODO
};

parser::parser( tag_caller_must_keep_this_string_valid x , const std::string & data_str)
 : m_data_begin( & * data_str.begin() ), m_data_now( m_data_begin ), m_data_end( & * data_str.end() )
{ }

parser::parser( tag_caller_must_keep_this_buffer_valid x , const char * buf , size_t size)
 : m_data_begin( buf ), m_data_now( m_data_begin  ), m_data_end( buf + size )
{ }

unsigned char parser::pop_byte_u() {
	if (! (m_data_now < m_data_end) ) throw format_error_read();
	if (! ((m_data_end - m_data_now) >= 1) ) throw format_error_read();
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	unsigned char c = *m_data_now;
	++m_data_now;
	return c;
}

signed char parser::pop_byte_s() {
	if (! (m_data_now < m_data_end) ) throw format_error_read();
	if (! ((m_data_end - m_data_now) >= 1) ) throw format_error_read();
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	signed char c = *m_data_now;
	++m_data_now;
	return c;
}

template <int S, typename T> T parser::pop_integer_u() {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	// TODO test if type T is not overflowing

	T value=0;
	uint64_t multiplier = 1LLU << ((S-1)*8); // TODO(r) style: LLU vs uint64_t
	for (auto i=0; i<S; ++i) {
		unsigned int this_byte = pop_byte_u();
		cerr << "Reading for S="<<S<<" this_byte = " << this_byte << " mul=" << multiplier << endl;
		value += this_byte * multiplier;
		multiplier = multiplier >> 8; // move to next 8 bits
	}
	return value;
}

template <int S, typename T> T parser::pop_integer_s() {
	throw std::runtime_error("Not implemented yet");
}

std::string parser::pop_bytes_n(size_t size) {
	if (size<1) throw format_error_read_badformat(); // 0 byte "reads" are invalid
	if (! (m_data_now < m_data_end) ) throw format_error_read(); // we run outside of string
	if (! (   static_cast<unsigned long long int>(m_data_end - m_data_now) >= size) ) throw format_error_read(); // the read will not fit // TODO(r) is this cast good?

	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size < m_data_end) );
	auto from = m_data_now;
	m_data_now += size; // *** move
	return std::string( from , size );
}



} // namespace


void test_trivialserialize() {
	cerr << endl<< "Tests: " << __FUNCTION__ << endl << endl;

	string f="fooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo";

	trivialserialize::generator gen(50);
	gen.push_byte_u(50);
	gen.push_byte_s(-42);
	gen.push_bytes_n(3, "abc");
	gen.push_integer_u<1>((unsigned int)150);
	gen.push_integer_u<2>((unsigned int)+30000);
	gen.push_integer_u<8>((unsigned int)+30000);
	gen.push_integer_u<4>((unsigned int)+2140000000);
	gen.push_integer_u<4>((unsigned int)+4294777777);
	gen.push_bytes_sizeoctets<1>("Octets1"+f, 100);
	gen.push_bytes_sizeoctets<2>("Octets2"+f, 100);
	gen.push_bytes_sizeoctets<3>("Octets3"+f, 100);
	gen.push_bytes_sizeoctets<4>("Octets4"+f, 100);

	cout << "Serialized: [" << gen.str() << "]" << endl;

	const string input = gen.str();
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , input );


	auto cu = parser.pop_byte_u();
	auto su = parser.pop_byte_s();
	cerr << "Read ["<<(int)cu<<"] and [" << (int)su << "]" << endl;

	std::string sa1 = parser.pop_bytes_n(3);	cerr << "Read ["<<sa1<<"]" << endl;

	cerr << "Number: " << parser.pop_integer_u<1,unsigned int>() << endl;
	cerr << "Number: " << parser.pop_integer_u<2,unsigned int>() << endl;
	cerr << "Number: " << parser.pop_integer_u<8,unsigned int>() << endl;
	cerr << "Number: " << parser.pop_integer_u<4,unsigned int>() << endl;
	cerr << "Number: " << parser.pop_integer_u<4,unsigned int>() << endl;

	auto sb1 = parser.pop_bytes_sizeoctets<1>();
	cerr<<"["<<sb1<<"]"<<endl;
	auto sb2 = parser.pop_bytes_sizeoctets<2>();
	cerr<<"["<<sb2<<"]"<<endl;
	auto sb3 = parser.pop_bytes_sizeoctets<3>();
	cerr<<"["<<sb3<<"]"<<endl;
	auto sb4 = parser.pop_bytes_sizeoctets<4>();
	cerr<<"["<<sb4<<"]"<<endl;

}


