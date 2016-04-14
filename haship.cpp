

#include "haship.hpp"

#include <sodium.h>


// c_haship_addr :
c_haship_addr::c_haship_addr() : std::array<unsigned char, g_haship_addr_size>({{}}) { }

c_haship_addr::c_haship_addr(tag_constr_by_hash_of_pubkey x, const c_haship_pubkey & pubkey ) : std::array<unsigned char, g_haship_addr_size>({{}}) {
	_info("Creating HIP from pubkey: " << pubkey);

	// TODO(r) [crypto] hashing of pubkey to HIP - review this
	unsigned char hash[crypto_generichash_BYTES];
	string_as_bin pubkey_bin( pubkey );
	_info("Creating HIP from pubkey: " << string_as_dbg(pubkey_bin).get());
	crypto_generichash(hash, sizeof hash, reinterpret_cast<const unsigned char*>(pubkey_bin.bytes.c_str()), pubkey_bin.bytes.size(), NULL, 0);
	//_info("...creating: " << string_as_dbg(string_as_bin( reinterpret_cast<const char*>(hash), sizeof hash)).get());

	crypto_generichash(hash, sizeof hash, hash, sizeof hash, NULL, 0);
	//_info("...creating: " << string_as_dbg(string_as_bin( reinterpret_cast<const char*>(hash), sizeof hash)).get());

	// TODO(r) repeat untill we "bruteforce" an fd42 ?

	at(0) = 0xfd;
	at(1) = 0x42;
	assert( sizeof hash >= 16 );
	for (size_t i=2; i<16; ++i) {
		at(i) = hash[i];
	}	
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_string x, const string & addr_string ) { ///< create the IP address from a string (as dot/colon IP notation)
	// "fd42:ff10..." -> array of bytes: 253, 66,   255, 16, ...
	//throw std::runtime_error(string("Not yet implemented:") + string(__FUNCTION__));

	if (addr_string.size() > (8*4+7)) throw std::invalid_argument("The IP address looks invalid (too long) on string ["+addr_string+"]"); // 8 groups of 4 hexchar, plus 7 collon
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
			if (exp>0) throw std::invalid_argument("Invalid address, with more then one '::' in it.");
			int cc=0; // count of colons
			for (char v : addr_string) if (v==':') ++cc;
			int add = gr_max - 1 - cc;
			for (int i=0; i<add; ++i) grtab.push_back("0000"); // expanding
			++exp;
		}
		if (gr.size()>4) throw std::invalid_argument("The group is invalid (wrong size) \"" +gr+"\"");
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

c_haship_addr::c_haship_addr(tag_constr_by_addr_bin x, const string_as_bin & data ) {
	assert( this->size() == data.bytes.size() );
	for (size_t i=0; i<this->size(); ++i) this->at(i) = data.bytes.at(i);
}

void c_haship_addr::print(ostream &ostr) const {
	string_as_hex dbg( string_as_bin(*this) );
	ostr << "hip:" << dbg.get();
}
ostream& operator<<(ostream &ostr, const c_haship_addr & v) {	v.print(ostr);	return ostr; }

void c_haship_pubkey::print(ostream &ostr) const {
	string_as_hex dbg( string_as_bin(*this) );
	ostr << "pub:" << dbg.get();
}
ostream& operator<<(ostream &ostr, const c_haship_pubkey & v) {	v.print(ostr);	return ostr; }



// c_haship_pubkey :
c_haship_pubkey::c_haship_pubkey() : std::array<unsigned char, g_haship_pubkey_size>({{}}) { }
c_haship_pubkey::c_haship_pubkey( const string_as_bin & input ) : std::array<unsigned char, g_haship_pubkey_size>({{}}) {
	for(size_t i=0; i<input.bytes.size(); ++i) at(i) = input.bytes.at(i);
//	for(auto v : input.bytes) at(
}
 

