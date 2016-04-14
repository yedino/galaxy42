#ifndef include_crypto_hpp
#define include_crypto_hpp

#include "libs1.hpp"

#include "strings_utils.hpp"

using namespace std;

namespace antinet_crypto {

class c_symhash_state {
	public:

		typedef string_as_bin t_hash;

		c_symhash_state( t_hash initial_state );
		void next_state( t_hash additional_secret_material = t_hash("") );
		t_hash get_password() const;

		t_hash Hash1( const t_hash & hash ) const;
		t_hash Hash2( const t_hash & hash ) const;
	
	private:
		t_hash m_state;
		int m_number; ///< for debug mostly for now
};

c_symhash_state::c_symhash_state( t_hash initial_state )
	: m_state( initial_state ) 
{
	_info("Initial state: " << m_state.bytes);
	_info("Initial state dbg: " << string_as_dbg(m_state).get() );
}

void c_symhash_state::next_state( t_hash additional_secret_material ) {
	m_state = Hash1( Hash1( m_state ) + additional_secret_material );
	_info("State:" << m_state.bytes);
	++m_number;
}

c_symhash_state::t_hash c_symhash_state::get_password() const {
	return Hash2( m_state );
}


c_symhash_state::t_hash c_symhash_state::Hash1( const t_hash & hash ) const {
	return string_as_bin( "a(" + hash.bytes + ")" );
}

c_symhash_state::t_hash c_symhash_state::Hash2( const t_hash & hash ) const {
	return string_as_bin( "B(" + hash.bytes + ")" );
}


void test_crypto() {
		
	_mark("Testing crypto");

	#define SHOW _info( symhash.get_password().bytes );

	c_symhash_state symhash( string_as_hex("6a6b") ); // "jk"
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state( string_as_bin("---RX-1---") );
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state( string_as_bin("---RX-2---") );
	SHOW;

	// SymHash




	// DH+DH


}

	

}


#endif

