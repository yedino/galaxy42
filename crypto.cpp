#include "crypto.hpp"

namespace antinet_crypto {

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

    // TODO I know this look horrible, we should implement some (unsigned char <-> char) wrapper
    size_t u_hashmsg_len = hash.bytes.length();
    const unsigned char* u_hashmsg = new unsigned char [u_hashmsg_len];
    u_hashmsg = reinterpret_cast<const unsigned char *>(hash.bytes.c_str());

    size_t out_u_hash_len = crypto_generichash_BYTES;
    unsigned char out_u_hash[crypto_generichash_BYTES];

    crypto_generichash(out_u_hash, sizeof out_u_hash_len,
                       u_hashmsg, u_hashmsg_len,
                       NULL, 0);

    return string_as_bin(reinterpret_cast<char *>(out_u_hash));
    //return string_as_bin( "a(" + hash.bytes + ")" );
}

c_symhash_state::t_hash c_symhash_state::Hash2( const t_hash & hash ) const {

    t_hash hash_from_hash = Hash1(hash);
    for(auto &ch : hash_from_hash.bytes) {
        ch = ~ch;
    }
    return Hash1(hash_from_hash);
    //return string_as_bin( "B(" + hash.bytes + ")" );
}



c_dhdh_state::c_dhdh_state(t_privkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub)
	: m_our_priv(our_priv), m_our_pub(our_pub), m_theirs_pub(theirs_pub)
{ }

void c_dhdh_state::step1() {
	m_r = secure_random();
	m_skp = execute_DH_exchange( m_our_priv , m_our_pub );
}

c_dhdh_state::t_symkey c_dhdh_state::execute_DH_exchange(t_privkey & my_priv , t_pubkey theirs_pub) {
	// TODO
	return t_symkey();
}

c_dhdh_state::t_symkey c_dhdh_state::secure_random() {
	return string_as_bin("RANDOM");
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


} // namespace