

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


// c_haship_pubkey :
c_haship_pubkey::c_haship_pubkey() : std::array<unsigned char, g_haship_pubkey_size>({}) { }
c_haship_pubkey::c_haship_pubkey( const string_as_bin & input ) : std::array<unsigned char, g_haship_pubkey_size>({}) {
	for(size_t i=0; i<input.bytes.size(); ++i) at(i) = input.bytes.at(i);
//	for(auto v : input.bytes) at(
}




