

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

c_haship_addr::c_haship_addr(tag_constr_by_addr_string x, const string & pubkey ) { ///< create the IP address from a string (as dot/colon IP notation)
	// "fd42:ff10..." -> array of bytes: 253, 66,   255, 16, ...
	//throw std::runtime_error(string("Not yet implemented:") + string(__FUNCTION__));

	// @TODO(r) review:
	vector<string> addr_oct(8,"0000");
	std::stringstream adr_s;
	adr_s << addr;
	string oct;
	// std::cout << addr << std::endl; // dbg
	const int octets = 8;
	for(int i = 0; i < octets; ++i) {
			getline(adr_s, oct, ':');
			// std::cout << oct << std::endl; // dbg
			while(oct.size() < 4) {
					oct.insert(0,1,'0');
			}
			addr_oct[i] = oct;
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
 

