#include "crypto.hpp"
#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include <sodiumpp/sodiumpp.h>

#include "ntru/include/ntru_crypto.h"


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

/*c_symhash_state::t_hash c_crypto_state::Hash1( const string & data ) const {
	return Hash1( string_as_bin( data ) );
}
*/
c_symhash_state::t_hash c_crypto_state::Hash1( const t_hash & hash ) const {

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

c_symhash_state::t_hash c_crypto_state::Hash2( const t_hash & hash ) const {

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

bool safe_string_cmp(const std::string & a, const std::string & b) {
	if (a.size() != b.size()) return false;
	return 0 == sodium_memcmp( a.c_str() , b.c_str() , a.size() );
}




void test_crypto() {



	// the goal:
	const string app_msg1("Message-send-from-application"); // the finall end-user text that we want to tunnel.
	string app_msg;
	for (int i=0; i<1; ++i) app_msg += app_msg1;

	// --- in runtime ---

	// X25519 DH exchange
	std::string Alice_dh_sk(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES)); // random secret key
	std::string Alice_dh_pk(sodiumpp::crypto_scalarmult_base(Alice_dh_sk));

	std::string Bob_dh_sk(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES));
	std::string Bob_dh_pk(sodiumpp::crypto_scalarmult_base(Bob_dh_sk));

	std::string Alice_dh_shared = sodiumpp::crypto_scalarmult(Alice_dh_sk, Bob_dh_pk);
	std::string Bob_dh_shared = sodiumpp::crypto_scalarmult(Bob_dh_sk, Alice_dh_pk);
	// TODO use generic hash
	if (Alice_dh_shared == Bob_dh_shared) _note("DH shared - OK"); else _erro("key exchange error");

	sodiumpp::nonce<crypto_box_NONCEBYTES> nonce;

	c_crypto_state crypto_state;

	// Alice prepare boxer
	// and xor pubkey_alice xor pubkey_bob TODO? (hash distribution)
	string Alice_dh_key = crypto_state.Hash1( string_as_bin(Alice_dh_shared) ).bytes.substr(0,crypto_secretbox_KEYBYTES);
	_note("Alice encrypts with: " << string_as_dbg(string_as_bin(Alice_dh_key)).get());
	assert( Alice_dh_pk != Bob_dh_pk ); // to avoid any tricks in this corner case when someone sends us back our pubkey
	typedef sodiumpp::nonce64 t_crypto_nonce;
	using sodiumpp::boxer_base;
	sodiumpp::boxer< t_crypto_nonce > Alice_boxer  ( boxer_base::boxer_type_shared_key() , Alice_dh_pk > Bob_dh_pk ,sodiumpp::encoded_bytes(Alice_dh_key, sodiumpp::encoding::binary) );
	sodiumpp::boxer< t_crypto_nonce > Alice_unboxer( boxer_base::boxer_type_shared_key() , Alice_dh_pk > Bob_dh_pk , sodiumpp::encoded_bytes(Alice_dh_key, sodiumpp::encoding::binary) );
	//string encrypt = sodiumpp::crypto_secretbox(app_msg, nonce.get().bytes, Alice_dh_key);

	// Bob  prepare boxer
	string Bob_dh_key = crypto_state.Hash1( string_as_bin(Bob_dh_shared) ).bytes.substr(0,crypto_secretbox_KEYBYTES);
	_note("Bob decrypts with: " << string_as_dbg(string_as_bin(Bob_dh_key)).get());
	assert( Bob_dh_key != Alice_dh_pk ); // to avoid any tricks in this corner case when someone sends us back our pubkey
	//string decrypt = sodiumpp::crypto_secretbox_open(encrypt, nonce.get().bytes, Bob_dh_key);
	sodiumpp::boxer< t_crypto_nonce > Bob_boxer  ( boxer_base::boxer_type_shared_key() , Bob_dh_pk > Alice_dh_pk , sodiumpp::encoded_bytes(Bob_dh_key, sodiumpp::encoding::binary ) );
	sodiumpp::boxer< t_crypto_nonce > Bob_unboxer( boxer_base::boxer_type_shared_key() , Bob_dh_pk > Alice_dh_pk , sodiumpp::encoded_bytes(Bob_dh_key, sodiumpp::encoding::binary));

	// Use CryptoAuth:

	auto msg_send = string{"Hello-world"};
	auto cypher = Alice_boxer.box(msg_send);
	//auto msg_recived = Bob_boxer.unbox(cypher);
	_info("send: " << msg_send );
	//_info("reci: " << msg_recived );
	//_info("cyph: " << cypher );

//	if (safe_string_cmp(app_msg,decrypt)) _note("Encrypted message - OK "); else _erro("Msg decoded differs!");
//	if (! safe_string_cmp(encrypt,decrypt)) _note("It is encrypted  - OK"); else _erro("Not encrypted?!");

	// XXX _note("Encrypted as:" << sodiumpp::bin2hex(encrypt));

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
