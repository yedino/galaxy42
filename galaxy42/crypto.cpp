#include "crypto.hpp"
#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include "ntru/include/ntru_crypto.h"

#include "trivialserialize.hpp"


namespace antinet_crypto {

t_crypto_system_type c_crypto_system::get_system_type() const { return e_crypto_system_type_invalid; }

t_crypto_system_type c_symhash_state::get_system_type() const { return e_crypto_system_type_symhash_todo; }

t_crypto_system_type c_multikeys_pub::get_system_type() const { return e_crypto_system_type_multikey_pub; }

t_crypto_system_type c_multikeys_PRIV::get_system_type() const { return e_crypto_system_type_multikey_private; }

t_crypto_system_type c_multikeys_PAIR::get_system_type() const { return e_crypto_system_type_multikey_private; }

// ==================================================================

bool c_multikeys_pub::operator>(const c_multikeys_pub &rhs) const {
	return this->get_hash() > rhs.get_hash();
}

std::string c_multikeys_pub::get_hash() const {
	if (m_hash_cached=="") update_hash();
	assert(m_hash_cached != "");
	return m_hash_cached;
}

void c_multikeys_pub::update_hash() const {
	string all_pub; // all public keys together

	// TODO

	/*

	// "aaa" "bbb" - 	"aaa;bbb"
  //  "aaa;bb" "b"

	for (size_t i = 0; i < m_cryptolists_pubkey.size(); ++i) {
		//if (m_cryptolists_pubkey.at(i).empty() || rhs.m_cryptolists_pubkey.at(i).empty()) continue;
		//return m_cryptolists_pubkey.at(0) > rhs.m_cryptolists_pubkey.at(0);
	}
	*/

	m_hash_cached="(hash-todo)";
}

std::string c_multikeys_pub::serialize_bin() const { ///< returns a string with all our data serialized, to a binary format
	trivialserialize::generator gen(100);

	/** 
	A sparse "map" of keys type => of vectors of strings

	format: there are 2 types of keys: ( 
		keys type 5:  ( 3 keys of this type: varstr varstr varstr )  
		keys type 42: ( 2 keys of this type: varstr varstr ) )  

	format: 2 ( 5 ( 3 varstr varstr varstr )  42 ( 2 varstr varstr ) )  

Example (as of commit after d3b9872f758a90541dde8fc2cf45a97b691e1a17)
Serialized pubkeys: [(104)[
0x02(02) - two key types
0x15(21) - first type is 21 (enum numbers changed for the test)
0x02(02) - two pieces of this key:
, , - it is a 32 char string
0x9f(159),0x9e(158),0x09(09),,+,V,0xc6(198),g,*,b,0xf3(243),w,a,,0x9d(157),0xf1(241),0xd6(214),0xdd(221),0x83(131),X,0xd0(208),0x97(151),0xbd(189),0xef(239),0xc6(198),z,U,0x97(151),0xd4(212),0x10(16),u,;
, ,0x11(17),3,\,0xe0(224),0xdc(220),0xc7(199),H,0xed(237),0x81(129),X,0xd1(209),0xca(202),=,A,0x15(21),0xd3(211),V,0xc0(192),&,0x90(144),",{,0xe4(228),0xaa(170),0x92(146),0xa2(162),0x85(133),0xd6(214),2,0xb7(183),0xcd(205),0x04(04),

0x17(23) - now key type 23,
0x01(01) - 1 piece of it
, ,f,0x80(128),0x81(129),$,0x98(152),0x89(137),0x0f(15),0xde(222),0xbc(188),0xbc(188),0x1f(31),g,0xc9(201),R,6,0x94(148),j,0xd2(210),0x8f(143),0xd5(213),0x98(152),q,0xb7(183),3,[,0xb2(178),0xb4(180),0x8c(140),0xa5(165),0xdf(223),.,b]]
	*/

	int used_types=0; // count how many key types are actually used
	for (unsigned int ix=0; ix<m_cryptolists_pubkey.size(); ++ix) { // for all key type (for each element)
		const vector<string> & pubkeys_of_this_system  = m_cryptolists_pubkey.at(ix); // take vector of keys
		if (pubkeys_of_this_system.size()) ++used_types; // << count them
	}

	gen.push_integer_uvarint(used_types); // save the size of crypto list (number of main elements)
	_info("Generating serialize - so far - " << to_debug(gen.str()));
	int used_types_check=0; // counter just to assert
	for (unsigned int ix=0; ix<m_cryptolists_pubkey.size(); ++ix) { // for all key type (for each element)
		const vector<string> & pubkeys_of_this_system  = m_cryptolists_pubkey.at(ix); // take vector of keys
		if (pubkeys_of_this_system.size()) { // save them this time 
			++used_types_check;
			assert(ix < std::numeric_limits<unsigned char>::max()); // save the type
			gen.push_integer_u<1>(ix);
			gen.push_vector_string( pubkeys_of_this_system ); // save the vector of keys
			_info("Generating serialize - so far - " << to_debug(gen.str()));
		}
	}
	assert(used_types_check == used_types); // we written same amount of keys as we previously counted
	return gen.str();
}

// ==================================================================

c_dhdh_state::t_symkey c_crypto_system::secure_random(size_t size_of_radom_data) const {
	// TODO memory-locking?
	t_symkey ret;
	ret.resize(size_of_radom_data);
	assert( ret.size() == size_of_radom_data );
	unsigned char *data_ptr = reinterpret_cast<unsigned char*>( & ret[0] );
	randombytes(data_ptr, ret.size());
	return ret;
}

// ==================================================================

c_symhash_state::c_symhash_state( t_hash initial_state )
	: m_state( initial_state )
{
//	_info("Initial state: " << m_state);
//	_info("Initial state dbg: " << string_as_dbg(m_state).get() );
}

void c_symhash_state::next_state( t_hash additional_secret_material ) {
	m_state = Hash1( Hash1( m_state ) + additional_secret_material );
	//_info("State:" << m_state.bytes);
	++m_number;
}

c_symhash_state::t_hash c_symhash_state::get_password() const {
	return Hash2( m_state );
}

c_symhash_state::t_hash c_crypto_system::Hash1( const t_hash & hash ) const {

    // TODO I know this look horrible, we should implement some (unsigned char <-> char) wrapper
    size_t u_hashmsg_len = hash.length();
    const unsigned char* u_hashmsg;
    u_hashmsg = reinterpret_cast<const unsigned char *>(&hash[0]);

   	const size_t out_u_hash_len = 64;
    assert( out_u_hash_len <=  crypto_generichash_BYTES_MAX );
    unsigned char out_u_hash[out_u_hash_len];

    crypto_generichash(out_u_hash, out_u_hash_len,
                       u_hashmsg, u_hashmsg_len,
                       nullptr, 0);

    return string(reinterpret_cast<char *>(out_u_hash),  out_u_hash_len);
}

c_symhash_state::t_hash c_crypto_system::Hash2( const t_hash & hash ) const {

    t_hash hash_from_hash = Hash1(hash);
    for(auto &ch : hash_from_hash) { // negate all octets in it
        ch = ~ch;
    }
    return Hash1(hash_from_hash);
    //return string_as_bin( "B(" + hash.bytes + ")" );
}

c_symhash_state::t_hash c_symhash_state::get_the_SECRET_PRIVATE_state() const {
	return m_state;
}

// ==================================================================



c_dhdh_state::c_dhdh_state(t_PRIVkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub)
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



c_dhdh_state::t_symkey c_dhdh_state::execute_DH_exchange(
const t_PRIVkey &my_priv, const t_pubkey &my_pub, const t_pubkey &theirs_pub)
{
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

c_crypto_system::t_symkey c_dhdh_state::execute_DH_exchange() {
	return execute_DH_exchange(m_our_priv, m_our_pub, m_theirs_pub);
}


std::pair<c_dhdh_state::t_pubkey, c_dhdh_state::t_PRIVkey> c_dhdh_state::generate_key_pair() {
	using namespace ecdh_ChaCha20_Poly1305;
	keypair_t keypair = generate_keypair();
	std::pair<c_dhdh_state::t_pubkey, c_dhdh_state::t_PRIVkey> ret;
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


// TODO(janek): case as enum name
// TODO(janek) fix identation
		std::string t_crypto_system_type_to_name(int val) {
			switch(val) {
				case 1:			return "X25519";
				case 2:			return "Ed25519";
				case 3:     return "ntru128";
				case 4:			return "geport_todo";
				case 5:			return "symhash_todo";
				case 6:			return "multikey";
					//default:		return "Wrong type";
			}
			return "UNKNOWN";
		}

char t_crypto_system_type_to_ID(int val) {
	switch(val) {
		case e_crypto_system_type_X25519: return 'x';
		case e_crypto_system_type_Ed25519: return 'e';
		case e_crypto_system_type_ntru128: return 't';
		case e_crypto_system_type_geport_todo: return 'g';
	}
	throw std::invalid_argument("[" + std::string(__func__) + "] Unknown crypto type (val == " + std::to_string(val) + ")");
}

t_crypto_system_type t_crypto_system_type_from_ID(char name) {
	switch(name) {
		case 'x': return e_crypto_system_type_X25519;
		case 'e': return e_crypto_system_type_Ed25519;
		case 't': return e_crypto_system_type_ntru128;
		case 'g': return e_crypto_system_type_geport_todo;
	}
	throw std::invalid_argument("[" + std::string(__func__) + "] Unknown crypto type (name == " + name + ")");
}


void c_multikeys_PAIR::debug() const {
	_info("KEY PAIR:");
	for (unsigned long ix=0; ix<m_pub.m_cryptolists_pubkey.size(); ++ix) {
		const auto & pubkeys_of_this_system  = m_pub. m_cryptolists_pubkey. at(ix);
		const auto & PRIVkeys_of_this_system = m_PRIV.m_cryptolists_PRIVkey.at(ix);
		_info("Cryptosystem: " << t_crypto_system_type_to_name(ix) );
		for(size_t iy=0; iy < m_pub.m_cryptolists_pubkey[ix].size(); ++iy){
			_info("  PUB:" << m_pub.m_cryptolists_pubkey[ix].at(iy) );
			_info("  PRIV:"<< m_PRIV.m_cryptolists_PRIVkey[ix].at(iy) << "\n");
		}
	}
	_info("---------");
}

void c_multikeys_PAIR::generate() {
	_info("Generting keypair");

	for (int i=0; i<2; ++i) {
	_info("X25519 generating...");
	std::string key_PRIV(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES)); // random secret key
	std::string key_pub(sodiumpp::crypto_scalarmult_base(key_PRIV)); // PRIV -> pub
	this->add_public_and_PRIVATE( e_crypto_system_type_X25519 , key_pub , key_PRIV );
	}

	for (int i=0; i<1; ++i) {
	_info("(fake) NTru generating..."); // XXX TODO
	std::string key_PRIV(sodiumpp::randombytes(crypto_scalarmult_SCALARBYTES)); // random secret key
	std::string key_pub(sodiumpp::crypto_scalarmult_base(key_PRIV)); // PRIV -> pub
	this->add_public_and_PRIVATE( e_crypto_system_type_ntru128 , key_pub , key_PRIV ); // XXX TODO test!
	}

	string serialized = this->m_pub.serialize_bin();
	_info("Serialized pubkeys: [" << to_debug(serialized) << "]");
}


void c_multikeys_pub::add_public(t_crypto_system_type type, const  t_pubkey & pubkey) {
	m_cryptolists_pubkey.at( type ).push_back( pubkey );
}

c_crypto_system::t_pubkey c_multikeys_pub::get_public(t_crypto_system_type crypto_type, size_t number_of_key) const {
	// TODO check range
	return m_cryptolists_pubkey.at(crypto_type).at(number_of_key);
}

void c_multikeys_PRIV::add_PRIVATE(t_crypto_system_type crypto_type,const t_PRIVkey & PRIVkey) {
	m_cryptolists_PRIVkey.at( crypto_type ).push_back( PRIVkey );
}

c_crypto_system::t_PRIVkey c_multikeys_PRIV::get_private(t_crypto_system_type crypto_type, size_t number_of_key) const {
	return m_cryptolists_PRIVkey.at(crypto_type).at(number_of_key);
}


void c_multikeys_PAIR::add_public_and_PRIVATE(t_crypto_system_type crypto_type,
	 const c_crypto_system::t_pubkey & pubkey ,
	 const c_crypto_system::t_PRIVkey & PRIVkey)
{
	m_pub.add_public(crypto_type, pubkey);
	m_PRIV.add_PRIVATE(crypto_type, PRIVkey);
}

bool safe_string_cmp(const std::string & a, const std::string & b) {
	if (a.size() != b.size()) return false;
	return 0 == sodium_memcmp( a.c_str() , b.c_str() , a.size() );
}


c_crypto_tunnel create_crypto_tunnel(c_multikeys_PAIR & self, c_multikeys_pub & other) {
	c_crypto_tunnel tunnel;

	//string a_dh = self.

	return tunnel;
}

c_crypto_system::t_symkey
c_stream_crypto::calculate_usable_key(const c_multikeys_PAIR & self,  const c_multikeys_pub & them) {
	// used in constructor!
	std::string dh_shared_part1 = sodiumpp::crypto_scalarmult(
		self.m_PRIV.get_private( e_crypto_system_type_X25519, 0),
		them.get_public(e_crypto_system_type_X25519, 0)
	);

	// TODO: and xor pubkey_alice xor pubkey_bob TODO? (hash distribution)
	string dh_shared_ready = self.m_PRIV.Hash1( dh_shared_part1 ).substr(0,crypto_secretbox_KEYBYTES);
	_info("DH based key to use: " << dh_shared_ready );

	return dh_shared_ready;
}

bool c_stream_crypto::calculate_nonce_odd(const c_multikeys_PAIR & self,  const c_multikeys_pub & them) {
	return self.m_pub > them;
}

t_crypto_system_type c_stream_crypto::get_system_type() const
{
	assert(false && "not implemented");
}

c_stream_crypto::c_stream_crypto(const c_multikeys_PAIR & self,  const c_multikeys_pub & them)
	:
	m_usable_key( calculate_usable_key(self, them) ), // calculate UK and save it, and now use it:
	m_nonce_odd( calculate_nonce_odd( self, them)),
	m_boxer(
		sodiumpp::boxer_base::boxer_type_shared_key(),
		m_nonce_odd ,
		sodiumpp::encoded_bytes(m_usable_key, sodiumpp::encoding::binary)
	),
	m_unboxer(
		sodiumpp::boxer_base::boxer_type_shared_key(),
		m_nonce_odd ,
		sodiumpp::encoded_bytes(m_usable_key, sodiumpp::encoding::binary)
	)
{
}

c_crypto_tunnel::c_crypto_tunnel(const c_multikeys_PAIR & self,  const c_multikeys_pub & them) {
	m_stream_crypto = make_unique<c_stream_crypto>( self , them );
}

void test_crypto() {

	// Create IDC:

	// Alice:
	c_multikeys_PAIR keypairA;
	keypairA.generate();

	// Bob:
	c_multikeys_PAIR keypairB;
	keypairB.generate();

	c_multikeys_pub keypubB = keypairB.m_pub;

	// Create KCT and CT (CTE?)
	c_crypto_tunnel AliceCT(keypairA, keypubB);

	return ;

#if 0
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

	c_crypto_system crypto_system;

	// Alice prepare boxer
	// and xor pubkey_alice xor pubkey_bob TODO? (hash distribution)
	string Alice_dh_key = crypto_system.Hash1( Alice_dh_shared ).substr(0,crypto_secretbox_KEYBYTES);
	_note("Alice encrypts with: " << string_as_dbg(string_as_bin(Alice_dh_key)).get());
	assert( Alice_dh_pk != Bob_dh_pk ); // to avoid any tricks in this corner case when someone sends us back our pubkey
	typedef sodiumpp::nonce64 t_crypto_nonce;
	using sodiumpp::boxer_base;

	sodiumpp::boxer< t_crypto_nonce > Alice_boxer  ( boxer_base::boxer_type_shared_key() ,
	(Alice_dh_pk > Bob_dh_pk) ,
	sodiumpp::encoded_bytes(Alice_dh_key, sodiumpp::encoding::binary)
	);

	sodiumpp::unboxer< t_crypto_nonce > Alice_unboxer( boxer_base::boxer_type_shared_key() , ! (Alice_dh_pk > Bob_dh_pk) , sodiumpp::encoded_bytes(Alice_dh_key, sodiumpp::encoding::binary) );
	_note("Alice boxer nonce: " << string_as_dbg(string_as_bin(Alice_boxer.get_nonce().get().bytes)).get());
	_note("Alice boxer nonce: " << string_as_dbg(string_as_bin(Alice_boxer.get_nonce_constant().bytes)).get());

	auto nonce_constant = Alice_boxer.get_nonce_constant();

	//string encrypt = sodiumpp::crypto_secretbox(app_msg, nonce.get().bytes, Alice_dh_key);

	// Use CryptoAuth:

	auto msg_send = string{"Hello-world"};
	auto cypher = Alice_boxer.box(msg_send);

	_info("send: " << msg_send );
	_info("cyph: " << cypher.to_binary() );


	// alice generate packet:
	trivialserialize::generator gen(50);
	// assert that message size is smaller then around 2^(8*2)

	gen.push_bytes_n(16, nonce_constant.bytes );
	gen.push_bytes_sizeoctets<1>( cypher.to_binary()  );
	gen.push_byte_u(';'); // XXX

	const auto & packet = gen.str();

	_info("Network packet:" << packet);
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() ,
		packet // !! do not change this while parser exists
	);
	const string Bob_nonce_constant_str = parser.pop_bytes_n(16);
	_info("Parsed: nonce const " << Bob_nonce_constant_str);
	// t_crypto_nonce Bob_nonce_constant( sodiumpp::encoded_bytes( Bob_nonce_constant_str , sodiumpp::encoding::binary ));
	const string Bob_cyphertext = parser.pop_bytes_sizeoctets<1>();
	_info("Parsed: cypher " << Bob_cyphertext);

	// Bob  prepare boxer:
	string Bob_dh_key = crypto_system.Hash1( Bob_dh_shared).substr(0,crypto_secretbox_KEYBYTES);
	_note("Bob decrypts with: " << string_as_dbg(string_as_bin(Bob_dh_key)).get());
	assert( Bob_dh_key != Alice_dh_pk ); // to avoid any tricks in this corner case when someone sends us back our pubkey
	//string decrypt = sodiumpp::crypto_secretbox_open(encrypt, nonce.get().bytes, Bob_dh_key);
	sodiumpp::boxer< t_crypto_nonce > Bob_boxer  ( boxer_base::boxer_type_shared_key() , (Bob_dh_pk > Alice_dh_pk) , sodiumpp::encoded_bytes(Bob_dh_key, sodiumpp::encoding::binary ) );

	sodiumpp::unboxer< t_crypto_nonce > Bob_unboxer(
		boxer_base::boxer_type_shared_key() ,
		! (Bob_dh_pk > Alice_dh_pk) ,
		sodiumpp::encoded_bytes(Bob_dh_key, sodiumpp::encoding::binary),
	  sodiumpp::encoded_bytes( Bob_nonce_constant_str , sodiumpp::encoding::binary)
	);
	_note("Bob unboxer nonce: " << string_as_dbg(string_as_bin(Bob_unboxer.get_nonce().get().bytes)).get());

	try {
		auto msg_recived = Bob_unboxer.unbox( sodiumpp::encoded_bytes(Bob_cyphertext,  sodiumpp::encoding::binary)  );
		_info("reci: " << msg_recived );
	} catch(const std::exception &e) {
		_erro("Failed: " << e.what());
	}

//	if (safe_string_cmp(app_msg,decrypt)) _note("Encrypted message - OK "); else _erro("Msg decoded differs!");
//	if (! safe_string_cmp(encrypt,decrypt)) _note("It is encrypted  - OK"); else _erro("Not encrypted?!");

	// XXX _note("Encrypted as:" << sodiumpp::bin2hex(encrypt));

	return;

#endif

	_mark("Testing crypto - unittests");
	if (! unittest::alltests() ) {
		_erro("Unit tests failed!");
		return ;
	}

	_mark("Testing crypto - more");

	#define SHOW _info( string_as_dbg( string_as_bin( symhash.get_password() ) ).get() );

	c_symhash_state symhash( string_as_hex("6a6b").get() ); // "jk"
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state( "---RX-1---" );
	SHOW;
	symhash.next_state();
	SHOW;
	symhash.next_state( "---RX-2---" );
	SHOW;

	// SymHash




	// DH+DH


}


} // namespace
