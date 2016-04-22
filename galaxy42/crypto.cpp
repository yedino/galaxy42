#include "crypto.hpp"
#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include <sodiumpp/sodiumpp.h>



namespace antinet_crypto {


c_dhdh_state::t_symkey c_crypto_state::secure_random(size_t size_of_radom_data) const {
	t_symkey ret;
	ret.bytes.resize(size_of_radom_data);
	assert(!ret.bytes.empty());
	unsigned char *data_ptr = reinterpret_cast<unsigned char *>(&ret.bytes[0]);
	randombytes(data_ptr, ret.bytes.size());
	return ret;
}

// ==================================================================

c_symhash_state::c_symhash_state( t_hash initial_state )
	: m_state( initial_state )
{
	_info("Initial state: " << m_state.bytes);
	_info("Initial state dbg: " << string_as_dbg(m_state).get() );
}

void c_symhash_state::next_state( t_hash additional_secret_material ) {
	m_state = Hash1( Hash1( m_state ) + additional_secret_material );
	//_info("State:" << m_state.bytes);
	++m_number;
}

c_symhash_state::t_hash c_symhash_state::get_password() const {
	return Hash2( m_state );
}


c_symhash_state::t_hash c_symhash_state::Hash1( const t_hash & hash ) const {

    // TODO I know this look horrible, we should implement some (unsigned char <-> char) wrapper
    size_t u_hashmsg_len = hash.bytes.length();
    const unsigned char* u_hashmsg;
    u_hashmsg = reinterpret_cast<const unsigned char *>(&hash.bytes[0]);

   	const size_t out_u_hash_len = 64;
    assert( out_u_hash_len <=  crypto_generichash_BYTES_MAX );
    unsigned char out_u_hash[out_u_hash_len];

    crypto_generichash(out_u_hash, out_u_hash_len,
                       u_hashmsg, u_hashmsg_len,
                       nullptr, 0);

    return string_as_bin(reinterpret_cast<char *>(out_u_hash),  out_u_hash_len);
}

c_symhash_state::t_hash c_symhash_state::Hash2( const t_hash & hash ) const {

    t_hash hash_from_hash = Hash1(hash);
    for(auto &ch : hash_from_hash.bytes) {
        ch = ~ch;
    }
    return Hash1(hash_from_hash);
    //return string_as_bin( "B(" + hash.bytes + ")" );
}

c_symhash_state::t_hash c_symhash_state::get_the_SECRET_PRIVATE_state() const {
	return m_state;
}

// ==================================================================



c_dhdh_state::c_dhdh_state(t_privkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub)
	: m_our_priv(our_priv), m_our_pub(our_pub), m_theirs_pub(theirs_pub)
{ }

void c_dhdh_state::step1() {
	m_r = secure_random(128); // TODO size
	m_skp = execute_DH_exchange( m_our_priv, m_our_pub, m_our_pub );
}

c_dhdh_state::t_pubkey c_dhdh_state::get_permanent_pubkey() const {
	return m_our_pub;
}

c_dhdh_state::t_pubkey c_dhdh_state::get_temp_pubkey() const {
	return m_pubkey_temp;
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

c_crypto_state::t_symkey c_dhdh_state::execute_DH_exchange() {
	return execute_DH_exchange(m_our_priv, m_our_pub, m_theirs_pub);
}


std::pair<c_dhdh_state::t_pubkey, c_dhdh_state::t_privkey> c_dhdh_state::generate_key_pair() {
	using namespace ecdh_ChaCha20_Poly1305;
	keypair_t keypair = generate_keypair();
	std::pair<c_dhdh_state::t_pubkey, c_dhdh_state::t_privkey> ret;
	ret.first.bytes.resize(keypair.pubkey.size());
	ret.second.bytes.resize(keypair.privkey.size());
	std::copy(keypair.pubkey.begin(), keypair.pubkey.end(), ret.first.bytes.begin());
	assert(keypair.pubkey.size() == ret.first.bytes.size());
	std::copy(keypair.privkey.begin(), keypair.privkey.end(), ret.second.bytes.begin());
	assert(keypair.privkey.size() == ret.second.bytes.size());
	return ret;
}


/*bool operator<( const c_symhash_state::t_hash &a, const c_symhash_state::t_hash &b) {
	return a.bytes < b.bytes;
}*/


namespace unittest {

// !!! WARNING:  most of the tests are now MOVED into googletest, e.g. into test/crypto.cpp !!!


// This will be probably removed soon from here - thugh it's a place to very quickly run some tests
// while you develop them

#define UTASSERT(X) do { if (!(X)) { _warn("Unit test failed!"); return false; } } while(0)
#define UTEQ(X,Y) do { if (!(X == Y)) { _warn("Unit test failed! Values differ: actuall=[" << X << "] vs expected=["<<Y<<"]" ); return false; } } while(0)

class c_symhash_state__tests_with_private_access {
	public:
		static bool foo1();
};

bool c_symhash_state__tests_with_private_access::foo1() {
	return true;
}

bool aeshash_foo2() {
	return true;
}

bool alltests() {
	if (! aeshash_foo2()) return false;
	if (! c_symhash_state__tests_with_private_access::foo1()) return false;
	return true;
}

} // namespace



void test_crypto() {


	const string app_msg("Message-send-from-application"); // the finall end-user text that we want to tunnel.
	const string key(crypto_secretbox_KEYBYTES, 'a');

	sodiumpp::nonce<crypto_box_NONCEBYTES> nonce;

	// encrypt
	string encrypt = sodiumpp::crypto_secretbox(app_msg, nonce.get().bytes, key);

	// decrypt
	string decrypt = sodiumpp::crypto_secretbox_open(encrypt, nonce.get().bytes, key);

	if (encrypt != decrypt) _note("OK "); else _erro("bad decrypt message");
	if ( app_msg == decrypt ) _note("OK "); else _erro("Msg decoded differs!");

	_note(sodiumpp::bin2hex(encrypt));

	// X25519 dh exchange
	std::string Alice_sk(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES)); // random secret key
	std::string Alice_pk(sodiumpp::crypto_scalarmult_base(Alice_sk));

	std::string Bob_sk(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES));
	std::string Bob_pk(sodiumpp::crypto_scalarmult_base(Bob_sk));

	std::string Alice_shared = sodiumpp::crypto_scalarmult(Alice_sk, Bob_pk);
	std::string Bob_shared = sodiumpp::crypto_scalarmult(Bob_sk, Alice_pk);
	// TODO use generic hash
	if (Alice_shared == Bob_shared) _note("OK "); else _erro("key exchange error");

	int SKPAB=0;
	_note("SKPAB="<<SKPAB);

	return;

	_mark("Testing crypto - unittests");
	if (! unittest::alltests() ) {
		_erro("Unit tests failed!");
		return ;
	}

	_mark("Testing crypto - more");

	#define SHOW _info( string_as_dbg( symhash.get_password() ).get() );

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
