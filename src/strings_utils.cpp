// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "strings_utils.hpp"

#include "crypto/crypto_basic.hpp" // for hashing function

#include "libs0.hpp"

// ==================================================================

string_as_hex::string_as_hex(const std::string & s) : data(s) { }

string_as_hex::string_as_hex(const string_as_bin & in) {
	size_t in_size = in.bytes.size();
	size_t size_mul = 2; // how much will the string expand
	size_t in_size_max1 = ( std::numeric_limits<decltype(in_size)>::max() / size_mul ) ; // it will expand e.g. x2 (that is not entire limit yet)
	_UNUSED(in_size_max1);
	assert( in_size < in_size_max1 );
	assert( in_size < (in_size_max1-1) ); // make sure no issue with ending C-string NULL
	size_t retsize = in_size*size_mul; // this will be size of output
	data.resize(retsize);

	size_t pos=0, outpos=0;
	for( ; pos<in_size; ++pos) {
		unsigned char byte = in.bytes.at(pos), bh=byte/16, bl=byte%16;
		data.at(outpos) = int2hexchar(bh);
		++outpos;
		data.at(outpos) = int2hexchar(bl);
		++outpos;
	}
	assert(outpos == retsize); // all expected out data was written
}

const std::string & string_as_hex::get() const { return data; }

std::ostream& operator<<(std::ostream &ostr, const string_as_hex &obj) { return ostr << obj.get(); }

bool operator==( const string_as_hex &a, const string_as_hex &b) {
	return a.data == b.data;
}

// ==================================================================


unsigned char int2hexchar(unsigned char i) {
	if (i<=9) return '0'+i;
	if (i<=15) return 'a'+(i-10);
	_throw_error( std::invalid_argument(  string("Invalid hex value:")+std::to_string(i) ) );
}

unsigned char hexchar2int(char c) {
	if ((c>='0')&&(c<='9')) return static_cast<unsigned char>(c-'0');
	if ((c>='a')&&(c<='f')) return static_cast<unsigned char>(c-'a' +10);
	_throw_error( std::invalid_argument(  string("Invalid character (")+string(1,c)+string(") in parsing hex number")  ) );
}

unsigned char doublehexchar2int(string s) {
	if (s.size()!=2) _throw_error( std::invalid_argument("Invalid double-hex string: '"+s+"'") );
	char h = s.at(0);
	char l = s.at(1);
	return hexchar2int(h)*16 + hexchar2int(l);
}

// ==================================================================

string_as_bin::string_as_bin(const std::string & bin)
	: bytes(bin)
{ }

string_as_bin::string_as_bin(const char * ptr, size_t size)
 : bytes(ptr,size)
{ }

string_as_bin::string_as_bin(const string_as_hex & encoded) {
try {
	// "ff020a" = ff , 02 , 0a
	//   "020a" = 02 , 0a
	//    "20a" = 02 , 0a
	const auto es = encoded.data.size();
	if (!es) return; // empty string encoded --> empty binary string

	size_t retsize = es/2; // size of finall string of bytes data
	if (0 != (es % 2)) retsize++;
	assert(retsize > 0); // the binary string will be not-empty (empty case is covered already)
	assert( (retsize < es)   ||   ((retsize==1)&&(es==1)) ); // binary string is smaller  -or-  both are ==1 for e.g. encoded "a" means "0a" so it should result in ---> a 1-byte binary string
	bytes.resize(retsize);

	size_t pos=0, out=0; // position of input, and output
	for( ; pos<es ; pos+=2, ++out) {
		// _info("pos="<<pos<<" out="<<out<<" encoded="<<encoded.data);
		// "02" -> cl="2" ch="0"
		//  "2" -> cl="2" ch="0"
		char cl,ch;
		if (pos+1 < es) { // pos and pos+1 are valid positions in string
			ch = encoded.data.at(pos);
			cl = encoded.data.at(pos+1);
		} else {
			ch = '0';
			cl = encoded.data.at(pos);
		}
		unsigned char octet = hexchar2int(ch)*16 + hexchar2int(cl);
		bytes.at(out) = octet;
	}

	assert( out == retsize ); // all expected positions of data allocated above in .resize() were written
} catch(std::exception &e) { _erro("Failed to parse string [" << encoded.data <<"]"); _throw_error_rethrow(  ); }
}

string_as_bin & string_as_bin::operator+=( const string_as_bin & other ) {
	bytes += other.bytes;
	return *this;
}
string_as_bin string_as_bin::operator+( const string_as_bin & other ) const {
	string_as_bin ret = *this;
	ret.bytes += other.bytes;
	return ret;
}

string_as_bin & string_as_bin::operator+=( const std::string & other ) {
	bytes += other;
	return *this;
}
string_as_bin string_as_bin::operator+( const std::string & other ) const {
	string_as_bin ret = *this;
	ret += other;
	return ret;
}

bool string_as_bin::operator==(const string_as_bin &rhs) const {
	return this->bytes == rhs.bytes;
}

bool string_as_bin::operator!=(const string_as_bin &rhs) const {
	return this->bytes != rhs.bytes;
}


bool operator<( const string_as_bin &a, const string_as_bin &b) {
	return a.bytes < b.bytes;
}

// ==================================================================

std::string chardbg(char c) {
	// this is quite slow
	std::ostringstream oss;
	string_as_dbg s;
	s.print(oss,c);
	return oss.str();
}

// ==================================================================

string_as_dbg::string_as_dbg(const string_as_bin & bin, t_debug_style style)
	: string_as_dbg( bin.bytes.begin() , bin.bytes.end() , style )
{ }

string_as_dbg::string_as_dbg(const char * data, size_t data_size, t_debug_style style)
	: string_as_dbg( string_as_bin( std::string(data,data_size) ) , style )
{ }

void string_as_dbg::print(std::ostream & os, char v, t_debug_style style)
{
	UNUSED(style); // TODONOW TODO
	unsigned char uc = static_cast<unsigned char>(v);
	signed char widthH=-1; // -1 is normal print, otherwise the width of hex
	signed char widthD; // width of dec
	if (uc<=9) {
		os << "0x" << static_cast<int>(uc);
	}
	else
	{
	if (uc<32) { widthH=2; widthD=2; }
	if (uc>127) { widthH=2; widthD=3; }
	if (widthH!=-1) { // escape it
		os << "0x" << std::hex << std::setfill('0') << std::setw(widthH) << std::uppercase << static_cast<int>(uc)
		   << '=' << std::dec << std::setfill('0') << std::setw(widthD) << static_cast<int>(uc);
	}
	else os<<v; // normal
	}
	//
}
void string_as_dbg::print(std::ostream & os, signed char v, t_debug_style style)
{ print(os, static_cast<char>(v), style); }

void string_as_dbg::print(std::ostream & os, unsigned char v, t_debug_style style)
{ print(os, static_cast<char>(v), style); }


string_as_dbg::operator const std::string & () const {
	return this->dbg;
}
const std::string & string_as_dbg::get() const {
	return this->dbg;
}

std::string to_debug(const std::string & data, t_debug_style style) {
	return string_as_dbg( string_as_bin( data ) , style ).get();
}

std::string to_debug(char data, t_debug_style style) {
	return to_debug( std::string(1,data) , style );
}

std::string to_debug(const string_as_bin & data, t_debug_style style) {
	return to_debug( data.bytes , style );
}


std::string to_debug_b(const std::string & data)
	{ return  to_debug(data,e_debug_style_big); }
std::string to_debug_b(const string_as_bin & data)
	{ return  to_debug(data,e_debug_style_big); }
std::string to_debug_b(char data)
	{ return  to_debug(data,e_debug_style_big); }

std::string debug_simple_hash(const std::string & str) {
	string s = antinet_crypto::Hash1(str).substr(0,4) ;
	string_as_bin bin( s );
	return  string_as_hex( bin ).get();
}



std::ostream & operator<<(std::ostream & os, boost::any & obj) {
	using namespace boost;
	try { os << any_cast<char>(obj); return os ; } catch(...){}
	try { os << any_cast<int>(obj); return os ; } catch(...){}
	try { os << any_cast<bool>(obj); return os ; } catch(...){}
	try { os << any_cast<float>(obj); return os ; } catch(...){}
	try { os << any_cast<double>(obj); return os ; } catch(...){}
	try { os << any_cast<string>(obj); return os ; } catch(...){}
	try { os << to_debug( any_cast<vector<string>>( obj ) );  return os ; } catch(...){}
	os << "(no-debug for this boost any type)";
	return os;
}

