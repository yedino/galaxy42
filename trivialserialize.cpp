
#include "libs1.hpp"

#include "trivialserialize.hpp"


using namespace std;

namespace trivialserialize {

const char * format_error::what() const noexcept { return "format-error in trivialserialize"; }

const char * format_error_read::what() const noexcept { return "format-error in trivialserialize while reading, input data is invalid"; }

const char * format_error_read_badformat::what() const noexcept { return "format-error in trivialserialize while reading, but it seems the format definition in the program is wrong"; }

const char * format_error_read_delimiter::what() const noexcept { return "format-error in trivialserialize while reading, input data is invalid, the delimiter was wrong."; }


const char * format_error_write::what() const noexcept { return "format-error in trivialserialize while writting, the given data can not be serialized"; }

const char * format_error_write_too_long::what() const noexcept { return "format-error in trivialserialize while writting, the given data can not be serialized - because data is too long (e.g. binary string)"; }

const char * format_error_write_value_too_big::what() const noexcept { return "format-error in trivialserialize while writing - value was too big over limit"; }


generator::generator(size_t suggested_size)
	: m_str()
{
	m_str.reserve( suggested_size );
}

void generator::push_byte_u(unsigned char c) { 	m_str += c; }
void generator::push_byte_s(signed char c) {	m_str += c; }


void generator::push_bytes_n(size_t size, const std::string & data) {
	assert(size == data.size()); // is the size of data the same as size that we think should go here
	m_str += data;
}

const std::string & generator::str() const { return m_str; }


// ==================================================================


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

void parser::pop_byte_skip(char c) { // read expected character (e.g. a delimiter)
	unsigned char was = pop_byte_u();
	unsigned char expected = static_cast<unsigned char>(c);
	if (was != expected) throw format_error_read_delimiter();
}

std::string parser::pop_bytes_n(size_t size) {
	if (size<1) throw format_error_read_badformat(); // 0 byte "reads" are invalid
	if (! (m_data_now < m_data_end) ) throw format_error_read(); // we run outside of string
	if (! (   static_cast<unsigned long long int>(m_data_end - m_data_now) >= size) ) throw format_error_read(); // the read will not fit // TODO(r) is this cast good?

	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	auto from = m_data_now;
	m_data_now += size; // *** move
	return std::string( from , size );
}


} // namespace

// ==================================================================

void trivialserialize::test_trivialserialize() {
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


