
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
	auto size = data.size(); // TODO const
//	assert( size <= ) ); // TODO
	push_integer_uvarint( data.size() );
	for (decltype(size) i = 0; i<size; ++i) push_varstring(data.at(i));
}

const std::string & generator::str() const { return m_str; }

const std::string & generator::get_buffer() const { return m_str; }

// ==================================================================


parser::parser( tag_caller_must_keep_this_string_valid, const std::string & data_str)
 : m_data_begin( & * data_str.begin() ), m_data_now( m_data_begin ), m_data_end( & * data_str.end() )
{ }

parser::parser( tag_caller_must_keep_this_buffer_valid, const char * buf , size_t size)
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
	if (!size) return string("");
	if (! (m_data_now < m_data_end) ) throw format_error_read(); // we run outside of string
	if (! (   static_cast<unsigned long long int>(m_data_end - m_data_now) >= size) ) throw format_error_read(); // the read will not fit // TODO(r) is this cast good?
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	auto from = m_data_now;
	m_data_now += size; // *** move
	return std::string( from , size );
}

void parser::skip_bytes_n(size_t size) {
	if (!size) return;
	if (! (m_data_now < m_data_end) ) throw format_error_read(); // we run outside of string
	if (! (   static_cast<unsigned long long int>(m_data_end - m_data_now) >= size) ) throw format_error_read(); // the read will not fit // TODO(r) is this cast good?
	assert( (m_data_now < m_data_end) && (m_data_now >= m_data_begin) );
	assert( (m_data_now + size <= m_data_end) );
	// auto from = m_data_now;
	m_data_now += size; // *** move
}


void parser::pop_bytes_n_into_buff(size_t size, char * buff) {
	if (! (m_data_now < m_data_end) ) throw format_error_read(); // we run outside of string
	if (! (   static_cast<unsigned long long int>(m_data_end - m_data_now) >= size) ) throw format_error_read(); // the read will not fit // TODO(r) is this cast good?
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
	auto size = pop_integer_uvarint(); // TODO const
	// assert( size <= (1LLU << 64LLU) ); // TODO
	for (decltype(size) i = 0; i<size; ++i) ret.push_back( pop_varstring() );
	return ret;
}


bool parser::is_end() const {
	return m_data_now >= m_data_end ;
}

void parser::debug() const {
	ostringstream oss;
	oss << " is at POS octet number:" << (size_t)(m_data_now - m_data_begin) << '.';
	oss << " now="<<(void*)m_data_now<<" range is [" << (void*)m_data_begin << ".." << (void*)m_data_end << ").";

	oss << "Next: ";
	for (int i=0; i<6; ++i)	{ 
		auto now=m_data_now+i;	
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


// ==================================================================


struct c_tank {
	int ammo;
	int speed;
	string name;
};
std::ostream& operator<<(std::ostream& out, const c_tank & t) {
	out << "[" << t.ammo << ' ' << t.speed << ' ' << t.name << "]";
	return out;
}
bool operator==(const c_tank & aaa, const c_tank & bbb) {
	return (aaa.ammo == bbb.ammo) && (aaa.speed == bbb.speed) && (aaa.name == bbb.name);
}
bool operator<(const c_tank & aaa, const c_tank & bbb) {
	if (aaa.ammo < bbb.ammo) return 1;
	if (aaa.ammo > bbb.ammo) return 0;
	if (aaa.speed < bbb.speed) return 1;
	if (aaa.speed > bbb.speed) return 0;
	if (aaa.name < bbb.name) return 1;
	if (aaa.name > bbb.name) return 0;
	return 0; // the same
}

vector<c_tank> get_example_tanks() {
	vector<c_tank> data = {
		{ 150, 60 , "T-64"} ,
		{ 500, 70 , "T-72"} ,
		{ 800, 80 , "T-80"} ,
		{ 2000, 90 , "Shilka"} ,
	};
	return data;
}

map<string, c_tank> get_example_tanks_map_location() {
	map<string, c_tank> data = {
		{ "Moscow", { 150, 60 , "T-64"} } ,
		{ "Puszkin" , { 500, 70 , "T-72"} },
	};
	return data;
}

map<c_tank, string> get_example_tanks_map_captain() {
	map<c_tank, string> data = {
		{ { 150, 60 , "T-64"} , "Pavlov" },
		{ { 900, 80 , "BMP"} , "A. Ramius" },
	};
	return data;
}

template <> void obj_serialize(const c_tank & data , trivialserialize::generator & gen) {
	gen.push_integer_uvarint(data.ammo);
	gen.push_integer_uvarint(data.speed);
	gen.push_varstring(data.name);
}

template <> c_tank obj_deserialize<c_tank>(trivialserialize::parser & parser) {
	c_tank ret;
	ret.ammo = parser.pop_integer_uvarint();
	ret.speed = parser.pop_integer_uvarint();
	ret.name = parser.pop_varstring();
	return ret;
}

namespace test {

namespace detail {


} // namespace detail
}


template <typename T> ostream& operator<<(ostream &ostr , const vector<T> & tab) {
	bool first=0; // first was done yet
	for (const auto & obj : tab) {
		if (first) cout << ", ";
		first=1;
		ostr<<obj;
	}
	return ostr;
}

template <typename TKey, typename TVal> ostream& operator<<(ostream &ostr , const map<TKey,TVal> & tab) {
	bool first=0; // first was done yet
	for (const auto & it : tab) {
		if (first) cout << ", ";
		first=1;
		ostr << (it.first) << ": {" << (it.second) << "}";
	}
	return ostr;
}


namespace test {


void test_shortstring_end() {
	trivialserialize::generator gen(50);

	vector<string> test_varstring = {
		string("ABC"),
		string(""),
	};
	for (auto val : test_varstring) gen.push_varstring(val);

	const string input = gen.str();
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , input );

	for (auto val_expected : test_varstring) {
		auto val_given = parser.pop_varstring();
		cerr<<"varstring decoded: [" << val_given << "] with size=" << val_given.size() << endl;
		bool ok = ( val_given == val_expected );
		if (!ok) throw std::runtime_error("Failed test for expected value " + (val_expected));
	}
}

void test_trivialserialize() {
	using namespace detail;

	test_shortstring_end();

	cerr << endl<< "Tests: " << __FUNCTION__ << endl << endl;

	string f="fooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo";

	trivialserialize::generator gen(50);
	gen.push_byte_u(50);
	gen.push_byte_s(-42);
	gen.push_bytes_n(3, "abc");
	gen.push_bytes_n(3, "xyz");
	gen.push_bytes_n(0, "");
	gen.push_bytes_n(6, "foobar");
	gen.push_integer_u<1>((unsigned int)150);
	gen.push_integer_u<2>((unsigned int)+30000);
	gen.push_integer_u<8>((unsigned int)+30000);
	gen.push_integer_u<4>((unsigned int)+2140000000);
	gen.push_integer_u<4>((unsigned int)+4294777777);
	gen.push_bytes_sizeoctets<1>("Octets1"+f, 100);
	gen.push_bytes_sizeoctets<2>("Octets2"+f, 100);
	gen.push_bytes_sizeoctets<3>("Octets3"+f, 100);
	gen.push_bytes_sizeoctets<4>("Octets4"+f, 100);
	gen.push_bytes_sizeoctets<1>("", 100);

	vector<uint64_t> test_uvarint1 = {1,42,100,250, 0xFD,     0xFE,      0xFF,  1000,     0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFA};
	// width should be:              {1,1,   1,  1,  1+2,     1+2,        1+2,  1+2,         1+2,        1+4,         1+8};
	// serialization should be:                      FD,0,FD  FD,0,FE FD,0,FF  FD,(1000) FD,FFFF  FE,FFFFFFF  FF,FFFFFFFFA
	test_uvarint1.push_back(0xDEADCAFEBEEF);
	for (auto val : test_uvarint1) gen.push_integer_uvarint(val);

	vector<string> test_varstring = {
		string("Hi."),
		string("Now empty string:"),
		string(""),
		string("Now empty string x5"),
		string(""),
		string(""),
		string(""),
		string(""),
		string(""),
		string("Hello!"),
		string(250,'x'),
		string(255,'y'),
		string(400,'z'),
	};
	for (auto val : test_varstring) gen.push_varstring(val);

	gen.push_vector_string( test_varstring );

	gen.push_vector_object( get_example_tanks() );

	gen.push_map_object( get_example_tanks_map_location() );
	gen.push_map_object( get_example_tanks_map_captain() );


	{
	std::map<std::string, std::string> input;
	input["aaa"] = "bbb";
	input["111"] = "2222222222222222222";
	input["0"] = "zzzzzzzzz";
	input["asdfas"] = "5567";
	input[";"] = "...";
	input[",./234"] = ";433334;43;34;34;2<>;";
	input["           "] = "htfthfft";
	input[R"(		  		)"] = "xyz";
	input["%"] = std::string();
	input[std::string()] = std::string();
	input[std::string(259, 'x')] = std::string(259, 'x');
	gen.push_map_object(input);
	}

	// ==============================================

	cout << "Serialized: [" << gen.str() << "]" << endl;
	cout << "Serialized: [" << string_as_dbg( string_as_bin( gen.str() )).get() << "]" << endl;

	// ==============================================
	// ==============================================

	const string input = gen.str();
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , input );

	auto cu = parser.pop_byte_u();
	auto su = parser.pop_byte_s();
	cerr << "Read ["<<(int)cu<<"] and [" << (int)su << "]" << endl;

	std::string sa1 = parser.pop_bytes_n(3);	cerr << "Read ["<<sa1<<"]" << endl;
	std::string sa2 = parser.pop_bytes_n(3);	cerr << "Read ["<<sa2<<"]" << endl;
	std::string sa3 = parser.pop_bytes_n(0);	cerr << "Read ["<<sa3<<"]" << endl;
	std::string sa4 = parser.pop_bytes_n(6);	cerr << "Read ["<<sa4<<"]" << endl;

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
	auto sb5 = parser.pop_bytes_sizeoctets<1>();
	cerr<<"["<<sb5<<"]"<<endl;

	for (auto val_expected : test_uvarint1) {
		auto val_given = parser.pop_integer_uvarint();
		cerr<<"uvarint decoded: " << val_given << endl;
		bool ok = ( val_given == val_expected );
		if (!ok) throw std::runtime_error("Failed test for expected value " + std::to_string(val_expected));
	}

	for (auto val_expected : test_varstring) {
		auto val_given = parser.pop_varstring();
		cerr<<"varstring decoded: [" << val_given << "] with size=" << val_given.size() << endl;
		bool ok = ( val_given == val_expected );
		if (!ok) throw std::runtime_error("Failed test for expected value " + (val_expected));
	}

	auto test_varstring_LOADED = parser.pop_vector_string();
	for (auto val_given : test_varstring_LOADED) {
		cerr<<"vector string decoded: [" << val_given << "] with size=" << val_given.size() << endl;
	}

	auto tanks = parser.pop_vector_object<c_tank>();

	_info("Vector tank: " << tanks);
	for(auto & t : tanks) _info(t);
	if ( tanks == get_example_tanks()) {	_info("Container deserialized correctly"); }
		else throw std::runtime_error("Deserialization failed");

	auto tanks_location = parser.pop_map_object<string, c_tank>();
	if ( tanks_location == get_example_tanks_map_location()) {	_info("Container deserialized correctly"); }
		else throw std::runtime_error("Deserialization failed");
	_info("Map tank: " << tanks_location);

	auto tanks_captain = parser.pop_map_object<c_tank,string>();
	if ( tanks_captain == get_example_tanks_map_captain()) {	_info("Container deserialized correctly"); }
		else throw std::runtime_error("Deserialization failed");
	_info("Map tank: " << tanks_captain);

	auto output = parser.pop_map_object<std::string, std::string>();
	_info("Output: " << output);


}

} // namespace


} // namespace


