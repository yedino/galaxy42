#include "crypto.hpp"
#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

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
    const unsigned char* u_hashmsg;
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
	m_r = secure_random(128); // TODO size
	m_skp = execute_DH_exchange( m_our_priv, m_our_pub, m_our_pub );
}

c_dhdh_state::t_symkey c_dhdh_state::execute_DH_exchange(const t_privkey &my_priv, const t_pubkey &my_pub, const t_pubkey &theirs_pub) {
	using namespace ecdh_ChaCha20_Poly1305;
	keypair_t my_keys;
	std::copy(my_priv.bytes.begin(), my_priv.bytes.end(), my_keys.privkey.begin());
	std::copy(my_pub.bytes.begin(), my_pub.bytes.end(), my_keys.pubkey.begin());
	pubkey_t theirs_pub_key;
	std::copy(theirs_pub.bytes.begin(), theirs_pub.bytes.end(), theirs_pub_key.begin());
	sharedkey_t sk = generate_sharedkey_with (my_keys, theirs_pub_key);
	t_symkey ret(std::string(sk.begin(), sk.end()));
	return ret;
}

void c_dhdh_state::generate_temp_key_pair() {
	using namespace ecdh_ChaCha20_Poly1305;
	keypair_t keypair = generate_keypair();
	std::copy(keypair.privkey.begin(), keypair.privkey.end(), m_privkey_temp.bytes.begin());
	std::copy(keypair.pubkey.begin(), keypair.pubkey.end(), m_pubkey_temp.bytes.begin());
}


c_dhdh_state::t_symkey c_dhdh_state::secure_random(size_t size_of_radom_data) {
	t_symkey ret;
	ret.bytes.resize(size_of_radom_data);
	unsigned char *data_ptr = reinterpret_cast<unsigned char *>(const_cast<char *>(ret.bytes.data())); // !!! const_cast
	randombytes(data_ptr, ret.bytes.size());
	return ret;
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