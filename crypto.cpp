#include "crypto.hpp"
#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"



namespace antinet_crypto {


c_dhdh_state::t_symkey c_crypto_state::secure_random(size_t size_of_radom_data) {
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

c_dhdh_state::t_pubkey c_dhdh_state::get_permanent_pubkey() {
	return m_our_pub;
}

c_dhdh_state::t_pubkey c_dhdh_state::get_temp_pubkey() {
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

void c_dhdh_state::generate_temp_key_pair() {
	using namespace ecdh_ChaCha20_Poly1305;
	keypair_t keypair = generate_keypair();
	std::copy(keypair.privkey.begin(), keypair.privkey.end(), m_privkey_temp.bytes.begin());
	std::copy(keypair.pubkey.begin(), keypair.pubkey.end(), m_pubkey_temp.bytes.begin());
}


/*bool operator<( const c_symhash_state::t_hash &a, const c_symhash_state::t_hash &b) {
	return a.bytes < b.bytes;
}*/


namespace unittest {

#define UTASSERT(X) do { if (!(X)) { _warn("Unit test failed!"); return false; } } while(0)
#define UTEQ(X,Y) do { if (!(X == Y)) { _warn("Unit test failed! Values differ: actuall=[" << X << "] vs expected=["<<Y<<"]" ); return false; } } while(0)

class c_symhash_state__tests_with_private_access {
	public:
		static bool aeshash_not_repeating_state_nor_password();
};

bool c_symhash_state__tests_with_private_access::aeshash_not_repeating_state_nor_password() {
	std::set< c_symhash_state::t_hash > used_hash;
	const int amount_iterations = 1000;

	typedef enum { RX_none=0, RX_constant, RX_same, RX_random , RX_END } type_RX;


	for (auto rx_type = RX_none ; rx_type < RX_END ; rx_type = static_cast<type_RX>(static_cast<int>(rx_type) + 1)  ) {
		c_symhash_state symhash( string_as_hex("") );

		auto rx_same = symhash.secure_random( 10 );

		switch (rx_type) { // first nextstate after creation of symhash
			case RX_none:	break; // here we do not do it, but must be then done in others to avoid collision
			case RX_constant:	symhash.next_state( string_as_bin("foo") ); break;
			case RX_same:	symhash.next_state( rx_same ); break;
			case RX_random:	symhash.next_state( symhash.secure_random( 2 ) ); break;
			default: assert(false);
		}


		for (int i=0; i<amount_iterations; ++i) {
			{
				auto result = used_hash.insert( symhash.get_the_SECRET_PRIVATE_state() );
				_info("Result: " << result.second);
				UTASSERT( result.second == true ); // inserted new one
			}
			{
				auto result = used_hash.insert( symhash.get_password() );
				UTASSERT( result.second == true ); // inserted new one
			}
			switch (rx_type) {
				case RX_none:	symhash.next_state(); break;
				case RX_constant:	symhash.next_state( string_as_bin("foo") ); break;
				case RX_same:	symhash.next_state( rx_same ); break;
				case RX_random:	symhash.next_state( symhash.secure_random( 2 ) ); break;
				default: assert(false);
			}
		}
	}

	_note("Ended this test");
	return true;
}

bool aeshash_start_and_get_same_passwords() {
	c_symhash_state symhash( string_as_hex("6a6b") ); // "jk"
	auto p = string_as_hex( symhash.get_password() );
	//	cout << "\"" << string_as_hex( symhash.get_password() ) << "\"" << endl;
	UTEQ( p.get() , "1ddb0a828c4d3776bf12abbe17fb4d82bcaf202a1b00b5b54e90db701303d69ce235f36d25c9fd1343225888e00abdc0e18c2036e86af9f3a90faf1abfefedf7" );
	symhash.next_state();

	p = string_as_hex( symhash.get_password() );
	UTEQ( p.get() , "72e4af0f04e2113852fd0d5320a14aeb2219d93ed710bc9bd72173b4ca657f37e4270c8480beb8fded05b6161d32a6450d4c3abb86023984f4f9017c309b5330" );

	symhash.next_state();
	p = string_as_hex( symhash.get_password() );
	UTEQ( p.get() , "8a986c419f1347d8ea94b3ad4b9614d840bb2dad2e13287a7a6cb5cf72232c3211997b6435f44256a010654d6f49e71517e46ce420a77f09f3a425eabaa99d8a" );
	return true;
}

bool alltests() {
	if (! aeshash_start_and_get_same_passwords()) return false;
	if (! c_symhash_state__tests_with_private_access::aeshash_not_repeating_state_nor_password()) return false;
	return true;
}

} // namespace


void test_crypto() {

	_mark("Testing crypto - unittests");

	if (! unittest::alltests() ) {
		_erro("Unit tests failed!");
		return ;
	}

	unittest::aeshash_start_and_get_same_passwords();

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
