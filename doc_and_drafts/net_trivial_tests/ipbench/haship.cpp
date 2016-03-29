

#include "haship.hpp"


// c_haship_addr :
c_haship_addr::c_haship_addr() : std::array<unsigned char, g_haship_addr_size>({}) { }

c_haship_addr::c_haship_addr(tag_constr_by_hash_of_pubkey x, const c_haship_pubkey & pubkey ) : std::array<unsigned char, g_haship_addr_size>({}) {
	at(0) = 0xfd;
	at(1) = 0x42;
	for (size_t i=0; i<8; ++i) { // TODO real hash!!!
		at(2+i) = pubkey.at(i);
		at(16-1-i) = pubkey.at(i);
	}	
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_string x, const string & addr_string ) { ///< create the IP address from a string (as dot/colon IP notation)
	// "fd42:ff10..." -> array of bytes: 253, 66,   255, 16, ...
	//throw std::runtime_error(string("Not yet implemented:") + string(__FUNCTION__));

	vector<string> grtab(8,"0000"); // group tab
	std::stringstream adr_s;
	adr_s << addr_string;
	const int gr_max = 8; // groups: maximum (and required) number
	// TODO handle "::"
	// TODO assert at most one occurance of "::"
	int exp=0; // how many times did we expand "::"
	for(int i = 0; i < gr_max; ++i) {
		string gr; // given group
		getline(adr_s, gr, ':');
		if (gr.size()==0) { // inside ::
		}
		if (gr.size()>4) throw std::invalid_argument("The group is invalid (wrong size) \"" +gr+"\"");
		while(gr.size() < 4) gr.insert(0,1,'0');
		grtab.at(i) = gr;
	}
	
	for(const string & gr: grtab) {
		_info("gr="<<gr);
		assert(gr.size() == 4);
	}


}

void c_haship_addr::print(ostream &ostr) const {
	string_as_dbg dbg( (*this) );
	ostr << dbg.get();
}

ostream& operator<<(ostream &ostr, const c_haship_addr & v) {
	v.print(ostr);
	return ostr;
}



// c_haship_pubkey :
c_haship_pubkey::c_haship_pubkey() : std::array<unsigned char, g_haship_pubkey_size>({}) { }
c_haship_pubkey::c_haship_pubkey( const string_as_bin & input ) : std::array<unsigned char, g_haship_pubkey_size>({}) {
	for(size_t i=0; i<input.bytes.size(); ++i) at(i) = input.bytes.at(i);
//	for(auto v : input.bytes) at(
}
 

