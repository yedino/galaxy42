// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt



#include "haship.hpp"
#include "strings_utils.hpp"

#include <sodium.h>


// c_haship_addr :
c_haship_addr::c_haship_addr()
	 : std::array<unsigned char, g_haship_addr_size>({{}})
{ }

c_haship_addr::c_haship_addr(tag_constr_by_hash_of_pubkey, const c_haship_pubkey & pubkey)
	 : std::array<unsigned char, g_haship_addr_size>({{}})
{
	_info("Creating HIP from pubkey: " << pubkey);

	string addr = pubkey.get_ipv6_string_bin();
	for (size_t i=0; i<16; ++i) at(i) = addr.at(i);
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_dot, const t_ipv6dot & addr_string ) {
	// "fd42:ff10..." -> array of bytes: 253, 66,   255, 16, ...
	//_throw_error( std::runtime_error(string("Not yet implemented:") + string(__FUNCTION__)) );

	if (addr_string.size() > (8*4+7)) _throw_error( std::invalid_argument("The IP address looks invalid (too long) on string ["+addr_string+"]") ); // 8 groups of 4 hexchar, plus 7 collon
	// now sting size is reasonable here, can operate on int

	vector<string> grtab; // group tab
	std::stringstream adr_s;
	adr_s << addr_string;
	const int gr_max = 8; // groups: maximum (and required) number
	// TODO handle "::"
	// TODO assert at most one occurance of "::"
	int exp=0; // how many times did we expand "::"
	while ( adr_s.good() ) {
		string gr; // given group
		getline(adr_s, gr, ':');
		//_info("gr="<<gr);
		if (gr.size()==0) { // inside ::
			if (exp>0) _throw_error( std::invalid_argument("Invalid address, with more then one '::' in it.") );
			int cc=0; // count of colons
			for (char v : addr_string) if (v==':') ++cc;
			int add = gr_max - 1 - cc;
			for (int i=0; i<add; ++i) grtab.push_back("0000"); // expanding
			++exp;
		}
		if (gr.size()>4) _throw_error( std::invalid_argument("The group is invalid (wrong size) \"" +gr+"\"") );
		while(gr.size() < 4) gr.insert(0,1,'0');
		grtab.push_back(gr);
	}
	//_note("Parsed as:");
	size_t pos=0;
	for(const string & gr: grtab) {
		//_info("gr="<<gr); // "fd42"
		assert(gr.size() == 4);

		unsigned char bh = doublehexchar2int( gr.substr(0,2) ); // "fd" -> 253
		unsigned char bl = doublehexchar2int( gr.substr(2,2) ); // "42" -> 66

		this->at(pos*2 + 0) = bh;
		this->at(pos*2 + 1) = bl;
		++pos;
	}
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_bin, const t_ipv6bin & data ) {
	if (! ( this->size() == data.size() ) ) {
		ostringstream oss; oss << "Trying to set hip address from binary data " << to_debug_b(data);
		_throw_error( std::runtime_error(oss.str()) );
	}
	for (size_t i=0; i<this->size(); ++i) this->at(i) = data.at(i);
}

void c_haship_addr::print(ostream &ostr) const {
	string_as_hex dbg( string_as_bin(*this) );
	ostr << "hip:" << dbg.get();
}
ostream& operator<<(ostream &ostr, const c_haship_addr & v) {	v.print(ostr);	return ostr; }

void c_haship_pubkey::print(ostream &ostr) const {
	ostr << "pub:" << ::to_debug(this->serialize_bin());
}
ostream& operator<<(ostream &ostr, const c_haship_pubkey & v) {	v.print(ostr);	return ostr; }



// c_haship_pubkey :
c_haship_pubkey::c_haship_pubkey() { }

c_haship_pubkey::c_haship_pubkey( const string_as_bin & input ) {
	_mark("Loadig pubkey from: " << ::to_debug(input) );
	this->load_from_bin(input.bytes);
	_mark("After loading: " << (*this) );
	//for(size_t i=0; i<input.bytes.size(); ++i) at(i) = input.bytes.at(i);
//	for(auto v : input.bytes) at(
}

