/**

GPL or other licence, see the licence file!

This is early pre-pre-alpha version, do NOT use it yet for anything,
do NOT run it other then in a test VM or better on isolated computer,
it has bugs and 'typpos'.
                                                 _       _
 _ __  _ __ ___       _ __  _ __ ___        __ _| |_ __ | |__   __ _
| '_ \| '__/ _ \_____| '_ \| '__/ _ \_____ / _` | | '_ \| '_ \ / _` |
| |_) | | |  __/_____| |_) | | |  __/_____| (_| | | |_) | | | | (_| |
| .__/|_|  \___|     | .__/|_|  \___|      \__,_|_| .__/|_| |_|\__,_|
|_|                  |_|                          |_|                
     _                       _                                    _   
  __| | ___      _ __   ___ | |_     _   _ ___  ___    _   _  ___| |_ 
 / _` |/ _ \    | '_ \ / _ \| __|   | | | / __|/ _ \  | | | |/ _ \ __|
| (_| | (_) |   | | | | (_) | |_    | |_| \__ \  __/  | |_| |  __/ |_ 
 \__,_|\___/    |_| |_|\___/ \__|    \__,_|___/\___|   \__, |\___|\__|
                                                       |___/          
 _                   _                     
| |__   __ _ ___    | |__  _   _  __ _ ___ 
| '_ \ / _` / __|   | '_ \| | | |/ _` / __|
| | | | (_| \__ \   | |_) | |_| | (_| \__ \
|_| |_|\__,_|___/   |_.__/ \__,_|\__, |___/
                                 |___/  

*/


#include "crypto.hpp"
#include "../crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include "../build_extra/ntru/include/ntru_crypto.h"
#include "sidhpp.hpp"

#include "../trivialserialize.hpp"

#include "../filestorage.hpp"
#include "../text_ui.hpp"
#include "ntrupp.hpp"

#include "../glue_lockedstring_trivialserialize.hpp"
#include "../glue_sodiumpp_crypto.hpp"

#include "multikeys.hpp"
#include "multikeys.tpl.hpp"

using sodiumpp::locked_string;

/**
 * @TODO
 * Things that need memlocked proving:
 * serialize_bin()
 * load_from_bin()
 * save() and load() - when calling e.g. serialize_bin() / load_from_bin(),
 * ... and them again, when doing file operations (fstream buffers - instead use simply 1 file function to read/write
 entire buffer at once (btw then size file size limit is it in reasonable values when loading etc)
 */


namespace antinet_crypto {



// ==================================================================
// c_symhash_state

t_crypto_system_type c_symhash_state::get_system_type() const { return e_crypto_system_type_symhash_todo; }

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

t_hash c_symhash_state::get_password() const {
	return Hash2( m_state );
}


t_hash c_symhash_state::get_the_SECRET_PRIVATE_state() const {
	return m_state;
}



// ==================================================================


c_stream::c_stream(bool side_initiator, const string& m_nicename)
:
	m_KCT( return_empty_K() ),
	m_nonce_odd( 0 ),
	m_side_initiator( side_initiator ),
	m_packetstart_kexasym(""),
	m_cryptolists_count(),
	m_boxer( nullptr ),
	m_unboxer( nullptr ),
	m_nicename(m_nicename)
{
	_dbg2n("created");
}

std::string c_stream::debug_this() const {
	ostringstream oss;
	oss << "{stream: "<<m_nicename<<"} ";
	return oss.str();
}

sodiumpp::locked_string c_stream::return_empty_K() {
	sodiumpp::locked_string ret( Hash1_size() );
	std::fill_n( ret.buffer_writable() , ret.size() , static_cast<char>(0));
	assert( ret.size() == Hash1_size() );
	return ret;
}


// ---------------------------------------------------------------------------

std::string c_stream::box(const std::string & msg) {
	auto & cb = * PTR(m_boxer); // my crypto (un)boxer
	const auto N = cb.get_nonce(); // nonce (before operation)
	const auto ret = cb.box(msg).to_binary();
	_dbg1n("Encrypt N="<<show_nice_nonce(N)
		<<" text " << to_debug(msg) << " ---> " << to_debug(ret)
		<<" K=" << to_debug_locked( cb.get_secret_PRIVATE_key()));
	return ret;
}

std::string c_stream::unbox(const std::string & msg) {
	auto & cb = * PTR(m_unboxer); // my crypto (un)boxer
	const auto N = cb.get_nonce(); // nonce (before operation)
	auto ret = cb.unbox(sodiumpp::encoded_bytes(msg , sodiumpp::encoding::binary));
	_dbg1n("Decrypt N="<<show_nice_nonce(N)
		<<" text " << to_debug(ret) << " <--- " << to_debug(msg)
		<<" K=" << to_debug_locked( cb.get_secret_PRIVATE_key()));
	return ret;
}

// ---------------------------------------------------------------------------

t_crypto_system_count c_stream::get_cryptolists_count_for_KCTf() const {
	return m_cryptolists_count;
}

void c_stream::exchange_start(const c_multikeys_PAIR & ID_self,  const c_multikeys_pub & ID_them,
	bool will_new_id)
{
	_noten("EXCHANGE START");
	m_KCT = calculate_KCT( ID_self , ID_them, will_new_id, "" );
	m_nonce_odd = calculate_nonce_odd( ID_self, ID_them );
	create_boxer_with_K();
}

void c_stream::exchange_done(const c_multikeys_PAIR & ID_self,  const c_multikeys_pub & ID_them,
			const std::string & packetstart)
{
	_noten("EXCHANGE DONE, packetstart=" << to_debug(packetstart));
	m_KCT = calculate_KCT( ID_self , ID_them, true, packetstart );
	m_nonce_odd = calculate_nonce_odd( ID_self, ID_them );
	create_boxer_with_K();
}

void c_stream::create_boxer_with_K() {
	_noten("Got stream K = " << to_debug_locked(m_KCT));
	sodiumpp::encoded_bytes nonce_zero =
		sodiumpp::encoded_bytes( string( t_crypto_nonce::constantbytes , char(0)), sodiumpp::encoding::binary)
	;
	m_boxer   = make_unique<t_boxer>  ( sodiumpp::boxer_base::boxer_type_shared_key(),   m_nonce_odd, m_KCT, nonce_zero );
	m_unboxer = make_unique<t_unboxer>( sodiumpp::boxer_base::boxer_type_shared_key(), ! m_nonce_odd, m_KCT, nonce_zero );
	_note("EXCHANGE start:: Stream Crypto prepared with m_nonce_odd=" << m_nonce_odd
		<< " and m_KCT=" << to_debug_locked( m_KCT )
		);
	_dbg1("EXCHANGE start: created boxer   with nonce=" << to_debug(PTR(m_boxer)  ->get_nonce().get().to_binary()));
	_dbg1("EXCHANGE start: created unboxer with nonce=" << to_debug(PTR(m_unboxer)->get_nonce().get().to_binary()));
	assert(m_boxer); assert(m_unboxer);
}

std::string c_stream::generate_packetstart(c_stream & stream_to_encrypt_with) const {
	trivialserialize::generator gen( m_packetstart_kexasym.size() + m_packetstart_IDe.size() + 20);
	_note("MAKING packetstart: m_packetstart_kexasym = " << to_debug(m_packetstart_kexasym));
	gen.push_varstring( m_packetstart_kexasym );

	_note("MAKING packetstart: m_packetstart_IDe = " << to_debug(m_packetstart_IDe));
	string packetstart_IDe_via_CT = stream_to_encrypt_with.box( m_packetstart_IDe );
	_note("MAKING packetstart: packetstart_IDe_via_CT = " << to_debug(packetstart_IDe_via_CT));
	gen.push_varstring( packetstart_IDe_via_CT );
	return gen.str();
}

string c_stream::parse_packetstart_kexasym(const string & data) const {
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , data );
 	auto ret = parser.pop_varstring(); // 1
	return ret;
}

string c_stream::parse_packetstart_IDe(const string & data) const {
	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , data );
	parser.skip_varstring(); // 1
	auto data_encr = parser.pop_varstring(); // 2
	auto data_decr = m_unboxer->unbox( sodiumpp::encoded_bytes(data_encr,sodiumpp::encoding::binary ));
	_info("Reading packetstart IDe, encr: " << to_debug(data_encr));
	_info("Reading packetstart IDe, decr: " << to_debug(data_decr));
	return data_decr;
}


unique_ptr<c_multikeys_PAIR> c_stream::create_IDe(bool will_asymkex) {
	_note("CREATING IDe (for my Tunnel probably)");
	unique_ptr<c_multikeys_PAIR> IDe = make_unique< c_multikeys_PAIR >();
	IDe -> generate( m_cryptolists_count , will_asymkex );
	m_packetstart_IDe = IDe->read_pub().serialize_bin(); // TODO(r) this should be all moved outside
	_dbg1("Created my IDe, ready to send it as: " << to_debug(m_packetstart_IDe) );
	return std::move(IDe);
}

void c_stream::set_packetstart_IDe_from(const c_multikeys_PAIR & keypair) {
	m_packetstart_IDe = keypair.read_pub().serialize_bin();
}

// ---------------------------------------------------------------------------

bool c_stream::calculate_nonce_odd(const c_multikeys_PAIR & self,  const c_multikeys_pub & them) {
	return self.m_pub > them;
}

t_crypto_system_type c_stream::get_system_type() const
{
	TODOCODE;	return t_crypto_system_type(0);
}

c_crypto_system::t_symkey c_stream::calculate_KCT
(const c_multikeys_PAIR & self, const c_multikeys_pub & them , bool will_new_id
, const std::string & packetstart )
{
	UNUSED(packetstart);
	//assert( self.m_pub.get_count_of_systems() == them.m_pub.get_count_keys_in_system() );
	assert(self.m_PRV.get_count_of_systems() == them.get_count_of_systems());
	// TODO assert self priv == them priv;
	assert(self.m_PRV.get_count_of_systems() == self.m_pub.get_count_of_systems());
	// TODO priv self == pub self

	typedef map< char , vector<string> > t_kexasym; // map for kexasym passwords grouped by crypto system

	t_kexasym kexasym_passencr_tosend; // passwords that I now generated for kexasym, encrypted to Bob
		// it will hold e.g. 't' => "ntrupassfoooo","ntrupassbr",   'r'=>"rsapass1",...   etc
	t_kexasym kexasym_passencr_received; // as above, but the ones I received from initiator

	if (m_side_initiator) {
		if (packetstart.size()) throw std::invalid_argument("Invalid use of CT: initiator mode, but not-empty packetstarter");
	}
	else {
		_dbg1("Parsing packetstart " << to_debug(packetstart));
		// I am respondent
		if (! packetstart.size()) throw std::invalid_argument("Invalid use of CT: not-initiator mode, but empty packetstarter");

		// TODO-speed: change to zerocopy view strings (after implemented in trivialserialize)
		string packetstart_kexasym = parse_packetstart_kexasym(packetstart);
		_dbg1("Parsing packetstart_kexasym " << to_debug(packetstart_kexasym));
		trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , packetstart_kexasym );
		kexasym_passencr_received = parser.pop_map_object<t_kexasym::key_type , t_kexasym::mapped_type>();
	}

	bool should_count = will_new_id;

	if (should_count) for (auto & count : m_cryptolists_count) count=0;

	// fill it with 0 bytes (octets):
	locked_string KCT_accum( Hash1_size() );
	for (size_t p=0; p<KCT_accum.size(); ++p) KCT_accum[p] = static_cast<unsigned char>(0);
	// TODO(rob): we could make locked_string(size_t, char) constructor and use it

	const c_multikeys_pub  & self_pub = self.m_pub ; // my    pub keys - all of this sys
	const c_multikeys_PRV & self_PRV = self.m_PRV; // my    PRV keys - all of this sys
	const c_multikeys_pub  & them_pub = them       ; // their pub keys - all of this sys

	for (size_t sys=0; sys<self.m_pub.get_count_of_systems(); ++sys) { // all key crypto systems
		// for given crypto system:

		auto sys_enum = int_to_enum<t_crypto_system_type>(sys); // enum of this crypto system

		auto key_count_a = self_pub.get_count_keys_in_system(sys_enum);
		auto key_count_b = them_pub.get_count_keys_in_system(sys_enum);
		auto key_count_bigger = std::max( key_count_a , key_count_b );

		if (key_count_bigger < 1) continue ; // !
		if (!( (key_count_a>0) && (key_count_b>0) )) continue ; // !

		auto sys_id = t_crypto_system_type_to_ID(sys_enum); // ID (e.g. char) of this crypto system
		_info("sys_id=" << sys_id);

		if (should_count) m_cryptolists_count.at(sys) = 1; // count that we use this cryptosystem

		if (sys == e_crypto_system_type_X25519) {
			_info("Will do kex in sys="<<t_crypto_system_type_to_name(sys)
				<<" between key counts: " << key_count_a << " -VS- " << key_count_b );

			for (decltype(key_count_bigger) keynr_i=0; keynr_i<key_count_bigger; ++keynr_i) {
				// if we run out of keys then wrap them around. this happens if e.g. we (self) have more keys then them
				auto keynr_a = keynr_i % key_count_a;
				auto keynr_b = keynr_i % key_count_b;
				_info("kex " << keynr_a << " " << keynr_b);

				auto const key_A_pub = self_pub.get_public (sys_enum, keynr_a);
				auto const key_A_PRV = self_PRV.get_PRIVATE(sys_enum, keynr_a);
				auto const key_B_pub = them_pub.get_public (sys_enum, keynr_b); // number b!

				_note("Keys:");
				_info(to_debug_locked_maybe(key_A_pub));
				_info(to_debug_locked_maybe(key_A_PRV));
				_info(to_debug_locked_maybe(key_B_pub));

				using namespace string_binary_op; // operator^

				// a raw key from DH exchange. NOT SECURE yet (uneven distribution), fixed below
				locked_string k_dh_raw( sodiumpp::key_agreement_locked( key_A_PRV, key_B_pub ) ); // *** DH key agreement (part1)
				_info("k_dh_raw = " << to_debug_locked(k_dh_raw) ); // _info( XVAR(k_dh_raw ) );

				locked_string k_dh_agreed = // the fully agreed key, that is secure result of DH
				Hash1_PRV(
					Hash1_PRV( k_dh_raw )
					^	Hash1( key_A_pub )
					^ Hash1( key_B_pub )
				);
				_info("k_dh_agreed = " << to_debug_locked(k_dh_agreed) );

				KCT_accum = KCT_accum ^ k_dh_agreed; // join this fully agreed key, with other keys
				_info("KCT_accum = " <<  to_debug_locked( KCT_accum ) );
			}
		} // X25519


		#if 1
		// TODO MERGEME
		if (sys == e_crypto_system_type_NTRU_EES439EP1) {
			_info("Will do kex in sys="<<t_crypto_system_type_to_name(sys_enum)
				<<" between key counts: " << key_count_a << " -VS- " << key_count_b );

			for (decltype(key_count_bigger) keynr_i=0; keynr_i<key_count_bigger; ++keynr_i) {
				auto pass_nr = keynr_i;

				// if we run out of keys then wrap them around. this happens if e.g. we (self) have more keys then them
				auto keynr_a = keynr_i % key_count_a;
				auto keynr_b = keynr_i % key_count_b;
				_info("kex " << keynr_a << " " << keynr_b);

				auto const key_A_pub = self_pub.get_public (sys_enum, keynr_a);
				auto const key_A_PRV = self_PRV.get_PRIVATE(sys_enum, keynr_a);
				auto const key_B_pub = them_pub.get_public (sys_enum, keynr_b); // number b!

				using namespace string_binary_op; // operator^

				if (m_side_initiator) {
					// I am initiator - so I create random passwords, and encrypt them for other side of stream
					const uint16_t random_len = 65; // because this much fits in this NTRU NTRU_EES439EP1
					sodiumpp::locked_string password_cleartext
						= sodiumpp::randombytes_locked(random_len); // <--- generate password

					// encrypt
					_dbg1("NTru password GENERATED: " << to_debug_locked(password_cleartext));
					_dbg2("NTru to pubkey " << to_debug(key_B_pub));
					string password_encrypted = ntrupp::encrypt(password_cleartext.get_string(), key_B_pub);
					_dbg1("random data encrypted as: " << to_debug(password_encrypted));

					kexasym_passencr_tosend[sys_id].push_back(password_encrypted); // store encrypted to send to Bob

					// calculate K so we know it too, before we throw away plaintext of passwords
					locked_string k_dh_agreed = // the fully agreed key, that is secure result of DH
					Hash1_PRV(
						Hash1_PRV( password_cleartext )
						^	Hash1( key_A_pub )
						^ Hash1( key_B_pub )
					);
					_info("k_dh_agreed = " << t_crypto_system_type_to_name(sys_enum) << ": " << to_debug_locked(k_dh_agreed) );

					KCT_accum = KCT_accum ^ k_dh_agreed; // join this fully agreed key, with other keys
				}
				else { // they encrypted rand data to me, I need to decrypt:
					string & encrypted = kexasym_passencr_received.at(sys_id).at(pass_nr);
					_info("Opening NTru KEX: from encrypted=" << to_debug(encrypted));
					sodiumpp::locked_string decrypted = ntrupp::decrypt<sodiumpp::locked_string>(encrypted, key_A_PRV);
					_info("Opening NTru KEX: from decrypted=" << to_debug_locked(decrypted));

					// TODO double code
					locked_string k_dh_agreed = // the fully agreed key, that is secure result of DH
					Hash1_PRV(
						Hash1_PRV( decrypted )
						^	Hash1( key_A_pub )
						^ Hash1( key_B_pub )
					);
					_info("k_dh_agreed = " << t_crypto_system_type_to_name(sys_enum) << ": " << to_debug_locked(k_dh_agreed) );

					KCT_accum = KCT_accum ^ k_dh_agreed; // join this fully agreed key, with other keys

				}
				_info("KCT_accum = " <<  to_debug_locked( KCT_accum ) );
			}
		} // NTRU_EES439EP1
		#endif


		if (sys == e_crypto_system_type_SIDH) {
			_info("Will do kex in sys="<<t_crypto_system_type_to_name(sys)
				<<" between key counts: " << key_count_a << " -VS- " << key_count_b );
			for (decltype(key_count_bigger) keynr_i=0; keynr_i<key_count_bigger; ++keynr_i) {
				// if we run out of keys then wrap them around. this happens if e.g. we (self) have more keys then them
				auto keynr_a = keynr_i % key_count_a;
				auto keynr_b = keynr_i % key_count_b;
				_info("kex " << keynr_a << " " << keynr_b);
				auto const key_self_pub = self_pub.get_public (sys_enum, keynr_a);
				auto const key_self_PRV = self_PRV.get_PRIVATE(sys_enum, keynr_a);
				auto const key_them_pub = them_pub.get_public (sys_enum, keynr_b); // number b!

				const auto dh_secret = sidhpp::secret_agreement(key_self_PRV, key_self_pub, key_them_pub);

				using namespace string_binary_op; // operator^
				locked_string k_dh_agreed = // the fully agreed key, that is secure result of DH
				Hash1_PRV(
					Hash1_PRV( dh_secret) // agreed-shared-key, hashed (it should include A+B parts of SIDH)
					^ Hash1( key_self_pub )	^	Hash1( key_them_pub ) // and hash of public keys too
				); // and all of this hashed once more
				_info("SIDH secret key: " << to_debug_locked(k_dh_agreed));

				KCT_accum = KCT_accum ^ k_dh_agreed; // join this fully agreed key, with other keys

				// key agreement
			}
		} // SIDH

	}

	t_hash_PRV KCT_ready_full = Hash1_PRV( KCT_accum );
	_info("KCT_ready_full = " << to_debug_locked( KCT_ready_full ) );
	assert( KCT_ready_full.size() >= crypto_secretbox_KEYBYTES ); // assert that we can in fact narrow the hash

	locked_string KCT_ready = substr( KCT_ready_full , crypto_secretbox_KEYBYTES); // narrow it to length of symmetrical key

	trivialserialize::generator gen(1000);
	gen.push_map_object( kexasym_passencr_tosend );
	m_packetstart_kexasym = gen.str();
	_note("KCT created packetstart_kexasym: " << to_debug(m_packetstart_kexasym) );

	_note("KCT ready exchanged: " << to_debug_locked( KCT_ready ) );
	return KCT_ready;
} // calculate_KCT

// ------------------------------------------------------------------

// TODO(rob) move to locked string ; and also vice-versa function with other order of args
bool operator==(const sodiumpp::locked_string & a, const std::string & b) {
	std::string b_cpy = b;
	sodiumpp::locked_string b_locked = sodiumpp::locked_string::move_from_not_locked_string(std::move(b_cpy));
	using namespace sodiumpp;
	return a == b_locked;
}

bool c_stream::is_K_not_empty() const {
	return !( m_KCT == return_empty_K() ); // constant time op==
}

// ==================================================================
// c_crypto_tunnel


// ------------------------------------------------------------------

c_crypto_tunnel::c_crypto_tunnel(const c_multikeys_PAIR & self,  const c_multikeys_pub & them,
const string& nicename)
	: m_side_initiator(true),
	m_IDe(nullptr), m_stream_crypto_ab(nullptr), m_stream_crypto_final(nullptr), m_nicename(nicename)
{
	_noten("Alice? Creating the crypto tunnel (we are initiator)");
	m_stream_crypto_ab = make_unique<c_stream>(m_side_initiator, m_nicename+"-CTab"); // TODONOW
	PTR(m_stream_crypto_ab)->exchange_start( self, them , true );
	_noten("Alice? Creating the crypto tunnel (we are initiator) - DONE");
}

c_crypto_tunnel::c_crypto_tunnel(const c_multikeys_PAIR & self, const c_multikeys_pub & them,
	const std::string & packetstart, const string & nicename )
	: m_side_initiator(false),
	m_IDe(nullptr), m_stream_crypto_ab(nullptr), m_stream_crypto_final(nullptr), m_nicename(nicename)
{
	_note("Bob? Creating the crypto tunnel (we are respondent)");
	m_stream_crypto_ab = make_unique<c_stream>(false, nicename+"-CTab");
	PTR(m_stream_crypto_ab)->exchange_done( self, them , packetstart ); // exchange for IDC is ready
	_mark("Ok exchange for AB is finalized");

	_note("Bob? Ok created our IDe...");
	this->create_IDe(); // here, because we counted keys from AB above
	_note("Bob? Ok created our IDe - DONE");

	// exchange for IDe is ready here:
	_note("Bob? Ok creating final stream");
	m_stream_crypto_final = make_unique<c_stream>(true, m_nicename+"-CTf"); // I am initiator for CTe

	c_multikeys_pub them_IDe;
	them_IDe.load_from_bin( m_stream_crypto_ab->parse_packetstart_IDe( packetstart ) );
	_info("Bob? From packetstart got IDe: " << them_IDe.to_debug() );
	m_stream_crypto_final->exchange_start( * this->m_IDe , them_IDe
		, true // there is no "CTee" ... uhh nope? TODO-now
		); // no packetstart - I am initiator of CTe

	m_stream_crypto_final->set_packetstart_IDe_from( * m_IDe ); // finall stream will send our IDe in packetstarter

	_mark("Bob? created packet starter for CTe...");
//	_mark("Bob? created packet starter for CTe : " << to_debug((m_stream_crypto_final)->generate_packetstart()));
	_note("Bob? Creating the crypto tunnel (we are respondent) - DONE");
}

// ---

std::string c_crypto_tunnel::debug_this() const {
	return "{CT: "+m_nicename+"} ";
}

void c_crypto_tunnel::create_CTf(const string & packetstart) {
	_info("Alice? Creating CTf from packetstart="<<to_debug(packetstart));
	c_multikeys_pub them_IDe;
	them_IDe.load_from_bin( PTR(m_stream_crypto_ab)->parse_packetstart_IDe(packetstart) );
	m_stream_crypto_final = make_unique<c_stream>(false, m_nicename+"-CTf"); // I am not initiator of this return-stream CTe
	m_stream_crypto_final -> exchange_done( * this->m_IDe , them_IDe , packetstart);
	_info("Alice? Creating CTf - done");
}

// ------------------------------------------------------------------

std::string c_crypto_tunnel::get_packetstart_ab() const {
	return PTR(m_stream_crypto_ab)->generate_packetstart( * PTR(m_stream_crypto_ab) );

}

std::string c_crypto_tunnel::get_packetstart_final() const {
	return PTR(m_stream_crypto_final)->generate_packetstart( * PTR(m_stream_crypto_ab) );
}

// ------------------------------------------------------------------

std::string c_crypto_tunnel::box(const std::string & msg) {
	return PTR(m_stream_crypto_final)->box(msg);
}

std::string c_crypto_tunnel::unbox(const std::string & msg) {
	return PTR(m_stream_crypto_final)->unbox(msg);
}

std::string c_crypto_tunnel::box_ab(const std::string & msg) {
	return PTR(m_stream_crypto_ab)->box(msg);
}

std::string c_crypto_tunnel::unbox_ab(const std::string & msg) {
	return PTR(m_stream_crypto_ab)->unbox(msg);
}

// ------------------------------------------------------------------

// : c_stream(IDC_self, IDC_them, rand_ntru_data, std::vector<std::string>()) // TODOdel

void c_crypto_tunnel::create_IDe() {
	_mark("Creating IDe");
	if (m_IDe) throw std::runtime_error("Tried to create IDe again, on a CT that already has one created.");
	//m_IDe = make_unique<c_multikeys_PAIR>();
	//m_IDe->generate( PTR(m_stream_crypto_ab)->get_cryptolists_count_for_KCTf() );
	m_IDe = PTR( m_stream_crypto_ab )->create_IDe( true );
	_info("My IDe:");
	m_IDe->debug();
	_mark("Creating IDe - DONE");
}


// ##################################################################
// ##################################################################
// ##################################################################
// tests

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


void test_string_lock() {
	_mark("Testing locked string operations");
	{
		locked_string a(3);
		assert(a.size() == 3);

		a[0]='x';	a[1]='y';	a[2]='z';
		assert(a.size() == 3);
		assert(a.c_str() == string("xyz"));

		char * ptr = a.buffer_writable();
		*(ptr+0) = 'X';		*(ptr+1) = 'Y';		*(ptr+2) = 'Z';
		assert(a.size() == 3);
		assert(a.c_str() == string("XYZ"));
	}
	{
		string s("pqr");
		locked_string a(s);
		assert(a.size() == 3);

		a[0]='x';	a[1]='y';	a[2]='z';
		assert(a.size() == 3);
		assert(a.c_str() == string("xyz"));

		char * ptr = a.buffer_writable();
		*(ptr+0) = 'X';		*(ptr+1) = 'Y';		*(ptr+2) = 'Z';
		assert(a.size() == 3);
		assert(a.c_str() == string("XYZ"));
	}

	vector<locked_string> vec;
	std::string s1("TestString");
	locked_string s = locked_string::move_from_not_locked_string(std::move(s1));
	vec.push_back(s);
	vec.push_back(s);
	vec.push_back(s);
	if (vec.at(2).get_string() != "TestString") throw std::runtime_error("Test failed - vector of locked strings");
//	return;
}

// ##################################################################
// ##################################################################
void test_crypto() {

	test_string_lock();


	// Alice: IDC
	_mark("Create IDC for ALICE");
	c_multikeys_PAIR keypairA;
	keypairA.generate(e_crypto_system_type_X25519,0);

	keypairA.generate(e_crypto_system_type_Ed25519,5);
	keypairA.generate(e_crypto_system_type_NTRU_EES439EP1,1);
	keypairA.generate(e_crypto_system_type_SIDH, 1);
	_note("ALICE has IPv6: " << to_debug(keypairA.get_ipv6_string_hex()));
	if (0) {
		keypairA.datastore_save_PRV_and_pub("alice.key");
		keypairA.datastore_save_PRV_and_pub("alice2.key");

		c_multikeys_PAIR loadedA;
		loadedA.datastore_load_PRV_and_pub("alice.key");
		loadedA.datastore_save_PRV_and_pub("alice.key.again");
	}

	// Bob: IDC
	_mark("Create IDC for BOB");
	c_multikeys_PAIR keypairB;
	keypairB.generate(e_crypto_system_type_X25519,0);

	keypairB.generate(e_crypto_system_type_Ed25519,5);
	keypairB.generate(e_crypto_system_type_NTRU_EES439EP1,1);
	keypairB.generate(e_crypto_system_type_SIDH, 1);
	_note("BOB has IPv6: " << to_debug(keypairB.get_ipv6_string_hex()));

	c_multikeys_pub keypubA = keypairA.m_pub;
	c_multikeys_pub keypubB = keypairB.m_pub;

	if (0) {
		// Check key save/restore (without saving to file)
		string keypubA_serialized = keypubA.serialize_bin();
		c_multikeys_pub keypubA_restored;
		keypubA_restored.load_from_bin( keypubA_serialized );
		_note("Serialize save/load test: serialized key to: " << to_debug(keypubA_serialized));
		if (keypubA.get_hash() == keypubA_restored.get_hash()) {
			_info("Seems to match");
		} else throw std::runtime_error("Test failed serialize save/load");
	}



	// test seding messages in CT sessions

	for (int ib=0; ib<1; ++ib) {
		_mark("Starting new conversation (new CT) - number " << ib);

		// Create CT (e.g. CTE?) - that has KCT
		_note("Alice CT:");
		c_crypto_tunnel AliceCT(keypairA, keypubB, "Alice"); // start, has KCT_ab
		AliceCT.create_IDe();
		string packetstart_1 = AliceCT.get_packetstart_ab();
		_info("SEND packetstart to Bob: " << to_debug(packetstart_1));

		_note("Bob CT:");
		c_crypto_tunnel BobCT(keypairB, keypubA, packetstart_1, "Bobby"); // start -> has
		string packetstart_2 = BobCT.get_packetstart_final();

		AliceCT.create_CTf(packetstart_2);

		_mark("Prepared tunnels (KCTf from KCTab)");

		//c_multikeys_pub keypairA_IDe_pub = AliceCT.get_IDe().m_pub;
		//c_multikeys_pub keypairB_IDe_pub = BobCT  .get_IDe().m_pub;

		//_warn("WARNING: KCTab - this code is NOT SECURE [also] because it uses SAME NONCE in each dialog, "
		//	"so each CT between given Alice and Bob will have same crypto key which is not secure!!!");
		for (int ia=0; ia<5; ++ia) {
			auto msg1s = AliceCT.box("Hello");
			auto msg1r = BobCT.unbox(msg1s);
			//_note("Message: [" << msg1r << "] from: " << to_debug(msg1s));

			auto msg2s = BobCT.box("Hello");
			auto msg2r = AliceCT.unbox(msg2s);
			//_note("Message: [" << msg2r << "] from: " << to_debug(msg2s));
		}
	}

		return; // !!!




/*
		_mark("Preparing for ephemeral KEX:");
		_note( to_debug( keypairA_IDe_pub.serialize_bin() ) );
		_note( to_debug( keypairB_IDe_pub.serialize_bin() ) );

		AliceCT.create_CTf( keypairB_IDe_pub );
		BobCT  .create_CTf( keypairA_IDe_pub );
*/

		// generate ephemeral keys


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
map_size
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

void test_crypto_benchmark(const size_t seconds_for_test_case) {
	_mark("test_crypto_benchmark");

	// X25519
	size_t generated_keys_x25519 = 0;
	auto start_point = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start_point < std::chrono::seconds(seconds_for_test_case)) {
		auto pair = c_multikeys_PAIR::generate_x25519_key_pair();
		++generated_keys_x25519;
	}
	auto stop_point = std::chrono::steady_clock::now();
	unsigned int x25519_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point - start_point).count();

	// ntru
	size_t generated_keys_ntru = 0;
	start_point = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start_point < std::chrono::seconds(seconds_for_test_case)) {
		auto pair = c_multikeys_PAIR::generate_nrtu_key_pair();
		++generated_keys_ntru;
	}
	stop_point = std::chrono::steady_clock::now();
	unsigned int ntru_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point - start_point).count();

	//sidh
	size_t generated_keys_sidh = 0;
	start_point = std::chrono::steady_clock::now();
	while (std::chrono::steady_clock::now() - start_point < std::chrono::seconds(seconds_for_test_case)) {
		auto pair = c_multikeys_PAIR::generate_sidh_key_pair();
		++generated_keys_sidh;
	}
	stop_point = std::chrono::steady_clock::now();
	unsigned int sidh_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop_point - start_point).count();

	_info("X25519");
	_info("Generated " << generated_keys_x25519 << " in " << x25519_ms << " ms");
	_info(static_cast<double>(generated_keys_x25519) / x25519_ms * 1000 << " key pairs per second");
	_info("NTRU");
	_info("Generated " << generated_keys_ntru << " in " << ntru_ms << " ms");
	_info(static_cast<double>(generated_keys_ntru) / ntru_ms * 1000 << " key pairs per second");
	_info("SIDH");
	_info("Generated " << generated_keys_sidh << " in " << sidh_ms << " ms");
	_info(static_cast<double>(generated_keys_sidh) / sidh_ms * 1000 << " key pairs per second");

}


} // namespace
