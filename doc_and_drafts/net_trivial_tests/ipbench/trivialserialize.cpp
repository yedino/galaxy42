

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
const char * format_error_read::what() const noexcept { return "format-error in trivialserialize while reading"; }


class generator {
	protected:
		std::string m_str; ///< the generated data so far
		
	public:
		generator(size_t suggested_size);

		void push_bytes_n(size_t size, const std::string & data); ///< always same size of this field, no need to encode the length
		void push_bytes_v(size_t max_size, const std::string & data); ///< need to encode the length. And we here set the limit of it's size

		void push_byte(unsigned char c);
		void push_byte(signed char c);
		void push_byte(char c);

		void push_byte_u(unsigned char c);
		void push_byte_s(signed char c);

		const std::string & str() const; ///< get the generated string. This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!
};

generator::generator(size_t suggested_size) 
	: m_str()
{
	m_str.reserve( suggested_size );
}

void generator::push_bytes_n(size_t size, const std::string & data) {
	assert(size == data.size()); // is the size of data the same as size that we think should go here
	m_str += data;
}

void generator::push_bytes_v(size_t max_size, const std::string & data) {
	auto size = data.size();
	assert( size < max_size );
	push_byte_u(size); // !
	m_str += data;
}

void generator::push_byte(unsigned char c) {	m_str += c; }
void generator::push_byte(signed char c) {	m_str += c; }
void generator::push_byte(char c) {	m_str += c; }

void generator::push_byte_u(unsigned char c) { 	m_str += c; }
void generator::push_byte_s(signed char c) {	m_str += c; }

const std::string & generator::str() const { return m_str; }


/***
@brief For prasing serialized data. It must ncelly THROW in case if input is in format other then expected, e.g. trying to read more data then possible.
It absolutelly must at least assert all possible internal errors resulting from input that is logically invalid in any way (though of course
it must be technically valid, e.g. not nullptr string).
And this asserts must be always-on, because this is for parsing unsafe external data. TODO(r) abort() that can not be disabled
*/

class parser {
	public:
		struct tag_caller_owns_string {} ; 
		struct tag_caller_owns_buf {} ; 

		// TODO(r): pick a guideline and use existing view idiom like that:
		const char * m_data_begin; ///< the begining of string
		const char * m_data_now; ///< current position (must be inside string, must be in range begin..end)
		const char * m_data_end;

		parser( tag_caller_owns_string x , const std::string & data_str);
		parser( tag_caller_owns_buf x , const char * buf , size_t size);

		unsigned char pop_byte_u();
		signed char pop_byte_s();

		std::string pop_bytes_n(size_t size);
		std::string pop_bytes_v();
		// TODO: provide

};

parser::parser( tag_caller_owns_string x , const std::string & data_str) 
 : m_data_begin( & * data_str.begin() ), m_data_now( m_data_begin ), m_data_end( & * data_str.end() )
{ }

parser::parser( tag_caller_owns_buf x , const char * buf , size_t size)
 : m_data_begin( buf ), m_data_now( m_data_begin  ), m_data_end( buf + size )
{ }

unsigned char parser::pop_byte_u() {
	if (! (m_data_now < m_data_end) ) throw format_error_read();
	if (! ((m_data_end - m_data_now) <= 1) ) throw format_error_read();
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	unsigned char c = *m_data_now;
	++m_data_now;
	return c;
}


} // namespace


void test_trivialserialize() {
	cerr << endl<< "Tests: " << __FUNCTION__ << endl << endl;

	trivialserialize::generator gen(50);
	gen.push_bytes_n(3, "abc");
	gen.push_bytes_v(250, "Hello");
	gen.push_byte_u(130);

	cout << "Serialized: [" << gen.str() << "]" << endl;

	const string input = gen.str();
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_owns_string() , input );

	std::string s1 = parsing.pop_byte_n



}


