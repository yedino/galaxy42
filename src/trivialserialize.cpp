// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "libs1.hpp"

#include "trivialserialize.hpp"

#include "strings_utils.hpp"


using namespace std;

namespace trivialserialize {

const char * format_error::what() const noexcept {
	return "format-error in trivialserialize"; }

const char * format_error_read::what() const noexcept {
	return "format-error in trivialserialize while reading, input data is invalid"; }

const char * format_error_read_badformat::what() const noexcept {
	return "format-error in trivialserialize while reading, but it seems the format definition in the program is wrong"; }

const char * format_error_read_delimiter::what() const noexcept {
	return "format-error in trivialserialize while reading, input data is invalid, the delimiter was wrong."; }


const char * format_error_write::what() const noexcept {
	return "format-error in trivialserialize while writting, the given data can not be serialized"; }

const char * format_error_write_too_long::what() const noexcept {
	return "format-error in trivialserialize while writting, the given data can not be serialized - "
	"because data is too long (e.g. binary string)"; }

const char * format_error_write_value_too_big::what() const noexcept {
	return "format-error in trivialserialize while writing - value was too big over limit"; }


const char * format_error_read_invalid_version::what() const noexcept {
	return "format-error in trivialserialize while reading - the given version/magic number is not allowed"; }


generator::generator(size_t suggested_size)
	: m_str()
{
	m_str.reserve( suggested_size );
}

void generator::push_byte_u(unsigned char c) { 	m_str += c; }
void generator::push_byte_s(signed char c) {	m_str += c; }


void generator::push_bytes_n(size_t size, const std::string & data) {
	_UNUSED(size);
	assert(size == data.size()); // is the size of data the same as size that we think should go here
	m_str += data;
}

void generator::push_integer_uvarint(uint64_t val) {
	// like in https://en.bitcoin.it/wiki/Protocol_documentation
	if (val < 0xFD) {	push_integer_u<1>(val);	}
	else if (val < 0xFFFF) { push_byte_u(0xFD); push_integer_u<2>(val); }
	else if (val < 0xFFFFFFFF) { push_byte_u(0xFE); push_integer_u<4>(val); }
	else { push_byte_u(0xFF); push_integer_u<8>(val); }
}

void generator::push_varstring(const std::string &data) {
	push_integer_uvarint(data.size()); // save the length varint
	push_bytes_n(data.size(),data); // save the data
}

void generator::push_vector_string(const vector<string> & data) {
	const auto size = data.size();
	assert(size <= std::numeric_limits<uint64_t>::max());
	push_integer_uvarint( data.size() );
	for (std::remove_cv<decltype(size)>::type i = 0; i<size; ++i) push_varstring(data.at(i));
}

const std::string & generator::str() const { return m_str; }

std::string && generator::str_move() { return std::move(m_str); }

const std::string & generator::get_buffer() const { return m_str; }

// ==================================================================


parser::parser( tag_caller_must_keep_this_string_valid, const std::string & data_str)
 : m_data_begin( & * data_str.begin() ), m_data_now( m_data_begin ), m_data_end( &data_str.back() +1 )
{ }

parser::parser( tag_caller_must_keep_this_buffer_valid, const char * buf , size_t size)
 : m_data_begin( buf ), m_data_now( m_data_begin  ), m_data_end( buf + size )
{ }

unsigned char parser::pop_byte_u() {
	if (! (m_data_now < m_data_end) ) _throw_error( format_error_read() );
	if (! ((m_data_end - m_data_now) >= 1) ) _throw_error( format_error_read() );
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	unsigned char c = static_cast<unsigned char>(*m_data_now);
	++m_data_now;
	return c;
}

signed char parser::pop_byte_s() {
	if (! (m_data_now < m_data_end) ) _throw_error( format_error_read() );
	if (! ((m_data_end - m_data_now) >= 1) ) _throw_error( format_error_read() );
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	signed char c = *m_data_now;
	++m_data_now;
	return c;
}

void parser::pop_byte_skip(char c) { // read expected character (e.g. a delimiter)
	unsigned char was = pop_byte_u();
	unsigned char expected = static_cast<unsigned char>(c);
	if (was != expected) {
		--m_data_now;
		_throw_error( format_error_read_delimiter() );
	}
}

std::string parser::pop_bytes_n(size_t size) {
	if (!size) return string("");
	if (! (m_data_now < m_data_end) ) _throw_error( format_error_read() ); // we run outside of string
	// casting below is ok, because std::ptrdiff_t is 64 bits signed value, and size_t is 64 bits unsigned value.
	// and we are sure that this ptrdiff > 0 because of earlier above condition
	if (! (   static_cast<size_t>(m_data_end - m_data_now) >= size) ) _throw_error( format_error_read() );
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	auto from = m_data_now;
	m_data_now += size; // *** move
	return std::string( from , size );
}

void parser::skip_bytes_n(size_t size) {
	if (!size) return;
	if (! (m_data_now < m_data_end) ) _throw_error( format_error_read() ); // we run outside of string
	// casting below is ok, because std::ptrdiff_t is 64 bits signed value, and size_t is 64 bits unsigned value.
	// and we are sure that this ptrdiff > 0 because of earlier above condition
	if (! ( static_cast<size_t>(m_data_end - m_data_now) >= size) ) _throw_error( format_error_read() );
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	// auto from = m_data_now;
	m_data_now += size; // *** move
}


void parser::pop_bytes_n_into_buff(size_t size, char * buff) {
	if (! (m_data_now < m_data_end) ) _throw_error( format_error_read() ); // we run outside of string
	// casting below is ok, because std::ptrdiff_t is 64 bits signed value, and size_t is 64 bits unsigned value.
	// and we are sure that this ptrdiff > 0 because of earlier above condition
	if (! ( static_cast<size_t>(m_data_end - m_data_now) >= size) ) _throw_error( format_error_read() );
	assert(buff!=nullptr);

	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	auto range1 = m_data_now;
	m_data_now += size; // *** move
	auto range2 = m_data_now;
	stdplus::copy_and_assert_no_overlap_size(range1, range2, buff, size); // copy the result into output
}

uint64_t parser::pop_integer_uvarint() {
	// like in https://en.bitcoin.it/wiki/Protocol_documentation
	unsigned char first = pop_byte_u();
	if (first < 0xFD) {	return first;	}
	else if (first == 0xFD) { return pop_integer_u<2,uint64_t>(); }
	else if (first == 0xFE) { return pop_integer_u<4,uint64_t>(); }
	else { assert(first==0xFF); return pop_integer_u<8,uint64_t>(); }
}

std::string parser::pop_varstring() { ///< Decode string of any length saved by push_varstring()
	size_t size = pop_integer_uvarint();
	assert( size <= SANE_MAX_SIZE_FOR_STRING );
	return pop_bytes_n(size);
}

void parser::skip_varstring() {
	size_t size = pop_integer_uvarint();
	assert( size <= SANE_MAX_SIZE_FOR_STRING );
	return skip_bytes_n(size);
}

vector<string> parser::pop_vector_string() {
	vector<string> ret;
	const auto size = pop_integer_uvarint();
	assert(size <= std::numeric_limits<uint64_t>::max());
	for (std::remove_cv<decltype(size)>::type i = 0; i<size; ++i) ret.push_back( pop_varstring() );
	//for (auto i = decltype(size){0}; i<size; ++i) ret.push_back( pop_varstring() );
	return ret;
}

bool parser::is_end() const {
	return m_data_now >= m_data_end ;
}

void parser::debug() const {
	ostringstream oss;
	oss << " is at POS octet number:" << (size_t)(m_data_now - m_data_begin) << '.';
	oss << " now="<<(void*)m_data_now<<" range is [" << (void*)m_data_begin << ".." << (void*)m_data_end << ").";

	const int dbg_lookahead=8; // show this many characters ahead
	oss << " Next: ";
	for (int i=0; i<dbg_lookahead; ++i)	{ 
		auto now=m_data_now+i;
		if (i>0) oss<<",";
		if ((now < m_data_end) && (now >= m_data_begin)) oss << chardbg(*now);
			else { oss<<"(END)"; break; }
	}

	_info("Parser " << (void*)this << oss.str());
}


template <> void obj_serialize(const std::string & data, trivialserialize::generator & gen) {
	gen.push_varstring(data);
}

template<> std::string obj_deserialize<std::string>(trivialserialize::parser & parser) {
	std::string ret = parser.pop_varstring();
	return ret;
}

template <> void obj_serialize(const char & data, trivialserialize::generator & gen) {	gen.push_byte_u(data); }
template <> char obj_deserialize<char>(trivialserialize::parser & parser) {	return parser.pop_byte_u(); }


template <> void obj_serialize(const std::vector<string> & data, trivialserialize::generator & gen) {
	gen.push_vector_object(data);
}
template <> std::vector<std::string> obj_deserialize<std::vector<string>>(trivialserialize::parser & parser) {
	return parser.pop_vector_object<string>();
}

template <typename T> ostream& operator<<(ostream &ostr , const vector<T> & tab) {
	bool first=0; // first was done yet
	for (const auto & obj : tab) {
		if (first) ostr << ", ";
		first=1;
		ostr<<obj;
	}
	return ostr;
}

template <typename TKey, typename TVal> ostream& operator<<(ostream &ostr , const map<TKey,TVal> & tab) {
	bool first=0; // first was done yet
	for (const auto & it : tab) {
		if (first) ostr << ", ";
		first=1;
		ostr << (it.first) << ": {" << (it.second) << "}";
	}
	return ostr;
}



} // namespace


