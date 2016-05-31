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





#ifndef include_crypto_hpp
#define include_crypto_hpp

#include "libs1.hpp"
#include <sodium.h>
#include "strings_utils.hpp"
#include "gtest/gtest_prod.h"
#include "ntru/include/ntru_crypto_drbg.h"
#include <sodiumpp/sodiumpp.h>
#include <SIDH.h>

/**
 * @defgroup antinet_crypto Antinet Crypto
 * @page cryptoglossary Crypto Glossary
 *
 * * \b kagr - is a process of key agreement (using either DH and it variants, or other systems)
 *
 * * \b PFS - perfrect forward security - is the security property that means data should be secure even if [all other]
 * 			  long-term keys are compromised. In this system we have kind of PFS created by IDP -> IDC (see below),
 * 			  but mainly this is done by using IDCE instead IDC, which are unique keys for pair alice/bob AND
 * 			  for each of their tunnels.
 *
 * * \b IDP - ID of user of this system (usually a multikey of many crypto public keys), that is Permanently his (see HIP).
 * * \b IDC - ID of user of this system (usually a multikey of many crypto public keys), that is Current in use by him.
 * 			  It is authorized by his IDP. In Cjdns, IDP == IDC (there is no separation).
 * * \b IDPtC - ID Permanent to Current -  is the full chain of crypto signatures, pubkeys, etc,
 * 			 	to prove ownership from HIP and IDP to given IDC.
 * * \b IDCE - New ephemeral ID (multikey) used only for this one crypto (to have PFS). It is authorized to someone by IDC.
 * ... also known as IDe (TODO: pick one :)
 *
 * * \b IDP - - - - signatures (IDPtC) - - - -> IDC
 * \n
 * * Overview of creating of the CT between two people known by their IPv6:\n
 * <pre>
 * Alice: IPv6 HIP <-- IDP ---(IDPtC)---> IDC              IDCE (of Alice)
 *                                         |                  |
 *                                         |                ,-+--(auth+encrypt)
 *                                         |               /          \
 *                                         V              /            \
 *                                        kagr --> KCTab <              > kagr ---> KCTf - symmetrical
 *                                         ^              \            /                  crypto stream
 *                                         |               \          /
 *                                         |                '-+--(auth+encrypt)
 *                                         |                  |
 * Bob:   IPv6 HIP <-- IDP ---(IDPtC)---> IDC              IDCE (of Bob)
 * </pre>
 *
 * * \b HIP - Hash IP - the IP (here IPv6 always), that is a hash of some public key(s) - of his IDP.
 *
 * * \b KCT - Key for CryptoTunnel - this name usually means KCTf:
 *
 * * \b KCTab - is a key for CT, that is always the same between same "Alice and Bob" (therefore the name "ab") - the same
 * 			 	under same two IDC - because it uses nonce==0, therefore it is insecure.
 *			 	It is insecure to send any predictable/guessable data, and content "encrypted" with it can be spoofed (because
 * 			 	it lacks property of PFS), therefore it should be used only to exchange (e.g. authorize) the KCTf key.
 *
 * * \b KCTf - is the ephemeral final key in CT, that is generated for given one CT each time even for same Alice/Bob,
 * 		  	   secure to use. This is the most low-level, current, low-level key that is directly used for given CT crypto.
 *
 * * Used for - authentication, and optionally also encryption.
 * 			   	Currently alwyas this is some Symmetrical Key.
 *
 * * \b CT - CryptoTunnel. Similar to CA (CryptoAuth) from Cjdns. Tunnel that is encrypted: possibly encrypted,
 * 			 and always authenticated.
 *
 * * \b CTE - CryptoTunnel Encrypted - a CT that in addition always is encrypted (and of course is authenticated)
 * * \b CTNE - CryptoTunnel NOT Encrypted - a CT that is NOT encrypted (and of course is authenticated)
 *
 * * \b CT-E2E - a CT established end-to-end for transmission of data, it is of type CTE.
 * * \b CT-P2P - a CT established peer-to-peer for forwarding of data further, so it is only CTNE.
 *
 * * pub - public key
 * * PRV - private key
 * * PAIR - pair of pub + PRV
 *
 * * KEX - key exchange: Alice and Bob do so that: Alice has Bob's key (e.g. shared symmetrical key) (or viceversa, or both)
 * * KEXASYM - KEX done with assymetrical keys, usually e.g. Alice creates passwords and encrypts it for/to Bob
 * * DH Diffie-Hellman exchange (or similar). Both parties know instantly the shared key when they see pubkey of other.
 * * DH+ASYM - other name for KEXASYM
 *
 * * GMK - Galaxy Multi Key - is the name of data format used
 * * in file format (and in serializaion) you can find magic headers:
 * * GMKaS - Galaxy Multi Key, in version "a", the Secret key (private key)
 * * GMKao - Galaxy Multi Key, in version "a", the open key (public key)


 Crypto elements overview:

c_multikeys_general<>
c_multikeys_pub
c_multikeys_PRV
c_multikeys_PAIR

=== Full exchange (PFS and asymkex) ===

Alice:
(IDC) -> do KEX(+ASYM) -> {created IDC_ali_ASYM_PASS} + count_keys + KCTab !
gen(count_keys) -> IDe_ali
send IDC_ali_ASYM_PASS via clear
send IDe_ali - via KCTab

Bob:
recv IDC_ali_ASYM_PASS via clear
(IDC) + IDC_ali_ASYM_PASS -> do KEX(+ASYM) -> KCTab !
gen(count_keys) -> IDe_bob
recv IDe_ali via KCTab
IDe_ali -> do KEX(+ASYM) -> {created IDe_bob_ASYM_PASS} + KCTf !!
send IDe_bob_ASYM_PASS via KCTab
send IDe_bob via KCTab

Alice:
recv IDe_bob_ASYM_PASS via KCTab
recv IDe_bob via KCTab
IDe_bob + IDe_bob_ASYM_PASS -> do KEX(+ASYM) -> KCTf !!

----

a bit older - TODO update this:

Alice:
  exchange_start( multikeys_pub , multikeys_PRV ) -> do DH; count_keys_IDe; gen random IDC_asymkex_pass ---> KCT(ab)
  create_IDe();
  exchange_start_get_packetstart() - IDe, IDC_asymkex_pass{encrypted to IDC-Bob}
A===>B

Bob:
  exchange_done( c_multikeys_pub , c_multikeys_PRV , start_packet ) -> do DH; count; decrypt IDC_asymkex_pass, ---> KCT(ab)
  // --- KCTab ready ---
  create_IDe( without_asymkex )
  exchange_start( IDe, IDe ) ---> do DH (IDe{no asymkex?}), [dont count], gen random IDe_asymkex_pass ---> KCT(f)
  exchange_start_get_packetstart() - IDe{no_asymkex?}, IDe_asymkex_pass{encrypted to IDe-Alice}
A<===B

Alice:
  exchange_done( c_multikeys_pub , c_multikeys_PRV , start_packet ) -> do DH; count; decrypt IDe_asymkex_pass ) --> KCT(f)

  // --- KCTf ready ---

c_stream - between simply 1 and 1 Multikey
	exchange_start() opt:  bool will_new_id (should count?) ---> KCT(*)
	exchange_done()
	create_IDe() opt: bool will_asymkex
	exchange_start_get_packet() - return IDe{and asymkex if any} , tosend_asymkex_pass

c_crypto_tunnel - advanced connection between 1-1 Multikey, but adding e.g. ephemeral keys IDe, (KCTab -> CKTf)
	construct( mk_pub , mk_PRV )
  	exchange_start( multikeys_pub , multikeys_PRV );
  	create_IDe();

 */

using namespace std;

/// @ingroup antinet_crypto
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace antinet_crypto {


// extra implementation tools
bool safe_string_cmp(const std::string & a, const std::string & b);


// === for debug ===
std::string to_debug_locked(const sodiumpp::locked_string & data);


// random functions
DRBG_HANDLE get_DRBG(size_t size);

uint8_t get_entropy(ENTROPY_CMD cmd, uint8_t *out);
/**
 * Generate "nbytes" random bytes and output the result to random_array
 * Returns CRYPTO_SUCCESS (=1) on success, CRYPTO_ERROR (=0) otherwise.
 */
CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array);

namespace unittest {
	class c_symhash_state__tests_with_private_access;
} // namespace


// Group: crypto utils, free functions etc

/**
 * @defgroup crypto_utils Crypto utils
 * @ingroup antinet_crypto
 * Usefull free crypto functions
 */
///@{

namespace string_binary_op {

typedef sodiumpp::locked_string LS;
typedef std::string NS;

template <class T1, class T2>
T1 binary_string_xor(T1 const & str1, T2 const& str2) {
	// WARNING: this function is written with less assertive code (e.g. without at()),
	// it MUST be checked against any errors if you would modify it.
	const auto size1 = str1.size();
	const auto size2 = str2.size();
	if (size1 != size2) throw std::runtime_error(
		string("Can not execute function ")	+ string(__func__) + string(" because different size: ")
		+ std::to_string(size1) + " vs " + std::to_string(size2) );

	// this is safe also for locked string:
	T1 ret( str1 );
	for (size_t i=0; i<size1; ++i) ret[i] ^= str2[i];
	// TODO: decltype(size1) without const

	assert(ret.size() == str1.size());	assert(str1.size() == str2.size());
	return ret;
}


std::string operator^(const std::string & str1, const std::string & str2);
sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const sodiumpp::locked_string & str2);
sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const std::string & str2_un);

} //namespace

///@}

// ==================================================================

/**
 * @defgroup hash_func Hashing functions
 * @ingroup antinet_crypto
 * Hashing functions group
 */
///@{
typedef std::string t_hash; ///< type of hash
typedef sodiumpp::locked_string t_hash_PRV; ///< type of hash that contains a private secure data (e.g. is memlocked)

t_hash Hash1( const t_hash & hash );
size_t Hash1_size(); ///< returns size (in octets) of the output of Hash1 function
t_hash Hash2( const t_hash & hash );
size_t Hash2_size(); ///< returns size (in octets) of the output of Hash1 function

t_hash_PRV Hash1_PRV( const t_hash_PRV & hash );
t_hash_PRV Hash2_PRV( const t_hash_PRV & hash );

///@}

// ==================================================================

// must match: t_crypto_system_type_to_name()
enum t_crypto_system_type : unsigned char {
	// 0 is reserved
	e_crypto_system_type_invalid = 0,
	e_crypto_system_type_X25519 = 5,
	e_crypto_system_type_Ed25519 = 6,
	e_crypto_system_type_NTRU_EES439EP1 = 7,
	e_crypto_system_type_SIDH = 8,
	e_crypto_system_type_geport_todo = 9,
	e_crypto_system_type_symhash_todo = 10,
	e_crypto_system_type_END_normal,

	e_crypto_system_type_multikey_pub = 29,
	e_crypto_system_type_multikey_private = 30,
	e_crypto_system_type_multisign = 31,
	e_crypto_system_type_END,
};

std::string t_crypto_system_type_to_name(int val);
std::string enum_name(t_crypto_system_type e);

char t_crypto_system_type_to_ID(int val);
t_crypto_system_type t_crypto_system_type_from_ID(char name);
bool t_crypto_system_type_is_asymkex(t_crypto_system_type sys); ///< is this type doing asymmetric KEX instead of D-H

// ------------------------------------------------------------------

/** Some state of some crypto system */
class c_crypto_system {
	public:
		typedef sodiumpp::locked_string t_symkey; //< type of symmetric key
		typedef sodiumpp::locked_string t_PRVkey; //< type of private key

		typedef std::string t_pubkey; //< type of public key

		typedef sodiumpp::nonce64 t_crypto_nonce; //< the type of nonce that we use

		virtual ~c_crypto_system()=default;

		// virtual t_symkey secure_random(size_t size_of_radom_data) const;

		virtual t_crypto_system_type get_system_type() const;
};

// ==================================================================

/** State of the SymHash (aka: ratchet?) */
// TODO: this is unused for now
class c_symhash_state final : public c_crypto_system {
	public:
		c_symhash_state( t_hash initial_state );
		void next_state( t_hash additional_secret_material = t_hash("") );
		t_hash get_password() const;
		virtual t_crypto_system_type get_system_type() const;

	private:
		t_hash get_the_SECRET_PRIVATE_state() const; //< used e.g. by some tests
		friend class unittest::c_symhash_state__tests_with_private_access;
		FRIEND_TEST(crypto, aeshash_not_repeating_state_nor_password);

	private:
		t_hash m_state;
		int m_number; ///< for debug mostly for now
};


// ##################################################################
// ##################################################################
// all about the multikeys

// ==================================================================

enum t_crypto_use : unsigned char {
	e_crypto_use_secret='S', // for secret key (PRIVATE key)
	e_crypto_use_open='o',  // for open key (public key)
	e_crypto_use_signature='n',  // for signature - the signature data (not a key actually)
	e_crypto_use_fingerprint='f',  // for fingerprinting - e.g. hashes of individual public keys
};

/// A type to count how may keys we have of given crypto system.
typedef std::array< int , e_crypto_system_type_END	> t_crypto_system_count;

/** All keys, of template type TKey, of given identity. It can be e.g. all public keys, or all private, etc.  */
template <typename TKey>
class c_multicryptostrings : public c_crypto_system {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multicryptostrings<TKey> t_self; ///< My own type (concretized ofcourse). Useful for shorter coding style.

		typedef std::array< vector< TKey > , e_crypto_system_type_END	> t_cryptolists_general; ///< templated!
		typedef TKey t_key; ///< concretized key type used in this templated class

		///< A "map" of [e.g.] public keys of given type, organized by their crypto type.
		///< example use: to get 50-th of our Ed25519 keys: m_cryptolists_pubkey[ e_crypto_system_type_Ed25519 ].at(50);
		t_cryptolists_general m_cryptolists_general; ///<  *** The main collection ("map") of the keys

		mutable string m_hash_cached; ///< Hash of all my [e.g.] public keys (a cache,
		// auto calculated by getters/cleared by setters - when m_autoupdate_hash)
		// empty "" means that it needs calculation. (this is the default value)

		void update_hash() const; ///< calculate the current m_hash and save it

		t_crypto_use m_crypto_use; ///< what is our key use: e.g. are we secret key / public key / are we the signature etc

	protected:
		/// @name Modifiers - general version. \n(that sould be wrapped in child class) @{
		void add_key(t_crypto_system_type crypto_type, const TKey & key); ///< append one more key
		/// @}

		/// @name Getters - general version. \n(that sould be wrapped in child class) @{
		TKey get_key(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}

	public:
		c_multicryptostrings(t_crypto_use secop);
		virtual ~c_multicryptostrings()=default;

		/// @name Getters: @{
		size_t get_count_keys_in_system(t_crypto_system_type crypto_type) const; ///< how many keys of given type
		size_t get_count_of_systems() const; ///< how many key types?
		string get_hash() const; ///< const, though it is allowed to update mutable field with cache of current hash
		/// @}

		/// @name save/load: @{
		/** Returns a string with all our data serialized, see load_from_bin() for details
		 */
		virtual std::string serialize_bin() const;

		/** Set this object to data loaded from string from serialize_bin().
		 * Will delete any prior data in this object.
		 * Will be always compatible with older/newer versions (across stable releases of the program)
		 * @warning Must remain compatible especially because this can change the resulting HIP address!
		*/
		virtual void load_from_bin(const std::string & data);

		void datastore_save(const string  & fname, bool overwrite = false) const; ///< Save this data to a file.
		void datastore_load(const string  & fname); ///< Replace all current data with data datastore_loaded from this file.
		void clear(); ///< Delete all current data.
		/// @}

		bool operator>(const t_self &rhs) const; ///< some sorting order, e.g. using the get_hash(). Used e.g. to
		/// pick even/odd nonce depending on comparing keys.

		virtual std::string to_debug() const;
};

// ==================================================================

template <typename TKey>
class c_multikeys_general : public c_multicryptostrings<TKey> {
	public:
		c_multikeys_general(t_crypto_use secop);

};


// ==================================================================
/** All pubkeys of given identity */
class c_multisign : public c_multicryptostrings< std::string > {
	public:
		c_multisign();
		virtual t_crypto_system_type get_system_type() const;
};


// ==================================================================
/** All pubkeys of given identity */
class c_multikeys_pub : public c_multikeys_general<c_crypto_system::t_pubkey> {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multikeys_general<t_pubkey>::t_cryptolists_general  t_cryptolists_pubkey;

	public:
		c_multikeys_pub();
		virtual t_crypto_system_type get_system_type() const;

		string get_ipv6_string() const; ///< IPV6 from hash of this key

		/// @name Modifiers - concretized version. \n Ready to use. @{
		void add_public(t_crypto_system_type crypto_type, const t_key & key); ///< append one more key
		/// @}

		/// @name Getters - concretized version. \n Ready to use. @{
		t_key get_public(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}
};

// ==================================================================
/** All PRIVATE keys of given identity */
class c_multikeys_PRV : public c_multikeys_general<c_crypto_system::t_PRVkey> {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multikeys_general<t_PRVkey>::t_cryptolists_general  t_cryptolists_PRVkey;

	public:
		c_multikeys_PRV();
		virtual t_crypto_system_type get_system_type() const;

		/// @name Modifiers - concretized version. \n Ready to use. @{
		void add_PRIVATE(t_crypto_system_type crypto_type, const t_key & key); ///< append one more key
		/// @}

		/// @name Getters - concretized version. \n Ready to use. @{
		t_key get_PRIVATE(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}

		bool operator>(const t_key &rhs) const; ///< TODO refer to base class
};

class c_multikeys_PAIR {
	public:
		// TODO move to private:
		c_multikeys_pub m_pub;
		c_multikeys_PRV m_PRV;

	public:
		virtual ~c_multikeys_PAIR() = default;

		string get_ipv6_string() const; ///< IPV6 from hash of this key

		///< generate from list of how many keys of given type we need
		void generate(t_crypto_system_count cryptolists_count, bool will_asymkex);
		void generate(t_crypto_system_type crypto_system_type, int count=1); ///< generate and save e.g. 3 X25519 keys
		void generate(); ///< generate the default set of keys
		static std::pair<sodiumpp::locked_string, std::string> generate_x25519_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_ed25519_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_nrtu_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_sidh_key_pair();


		/**
		 * @brief multi_sign Multiple signing message using all keys of given sign type
		 * @param msg Massage that will be sign
		 * @param sign_type	Type of crypto system
		 * @return Vector of signs
		 */
		std::vector<std::string> multi_sign(const std::string &msg,
											t_crypto_system_type sign_type = e_crypto_system_type_Ed25519);
		/**
		 * @brief multi_sign_verify	Verify message that was signed multiple by c_multikeys_pub.
		 * 		  Throw sodiumpp::crypto_error when verify fails.
		 * @param msg Message that was signed.
		 * @param signs	Vector of signs
		 * @param pubkeys c_multikeys_pub that contain according to signs vector of public keys.
		 * 		  Public keys must coresponding to PRV keys that was used to sign message
		 * @param sign_type	Type of crypto system.
		 */
		static void multi_sign_verify(const std::string &msg,
							   const std::vector<std::string> &signs,
							   const c_multikeys_pub &pubkeys,
							   t_crypto_system_type sign_type = e_crypto_system_type_Ed25519);

		void debug() const;

		void add_public_and_PRIVATE(t_crypto_system_type crypto_type,
			const c_crypto_system::t_pubkey & pubkey ,
			const c_crypto_system::t_PRVkey & PRVkey);

		/// like c_multikeys_general<>::datastore_save(), for the private (+public) key
		void datastore_save_PRV_and_pub(const string  & fname_base) const;
		/// like c_multikeys_general<>::datastore_save(), for the public key
		void datastore_save_pub(const string  & fname_base) const;
		/// like c_multikeys_general<>::datastore_load(), for the private (+public) key
		void datastore_load_PRV_and_pub(const string  & fname_base);

		virtual t_crypto_system_type get_system_type() const;

		const c_multikeys_pub & read_pub() const;

};

/**
 * Crypto primitives are provided by - sodiumpp library (and NTru, SIDH, Geport - in future - TODO)
 * Raw symmetric encryption - done by c_stream
 * Adding needed crypto data - nonce constant, nonce counter - done by c_crypto_tunnel
 * Downloading pubkeys, adding other meta-data, transport - are to be done by other, higher layers.
 */

/**
 * Basic KCT crypto system of the stream, e.g. a KCTab / KCTf.
 * It does only the main crypto algorithm (or even it's part - without ephemeral)
 */
class c_stream final /* because strange ctor init list functions */
: public c_crypto_system
{
	private:
		t_symkey m_KCT; ///< the main KCT for this stream

		bool m_nonce_odd; ///< is our key uneven (odd) as used in sodiumpp to decide nonce for us
		// TODO lock it's memory before setting it!!!

		bool m_side_initiator; ///< am I initiator of the dialog (or respondent); This is usually copied from my CT usually

		///@{
		/// @name Extra data created as result of KEX (or as part of protocol e.g. IDe)
		std::string m_packetstart_kexasym; ///< Generated by me data for packet-start kexasym (e.g. NTru)
		std::string m_packetstart_IDe; ///< data to connect to our new IDe (usually it's pubkey)

		t_crypto_system_count m_cryptolists_count; ///< Our count: how many keys we have of each crypto system
		///@}

		typedef sodiumpp::boxer< t_crypto_nonce > t_boxer;
		typedef sodiumpp::unboxer< t_crypto_nonce > t_unboxer;

		// Objects to use the stream:
		unique_ptr< t_boxer > m_boxer;
		unique_ptr< t_unboxer > m_unboxer;

		string m_nicename; ///< my nice name for logging/debugging

	public:
		c_stream(bool side_initiator, const string& m_nicename);
		std::string debug_this() const;

		void exchange_start(const c_multikeys_PAIR & ID_self,  const c_multikeys_pub & ID_them,
			bool will_new_id);

		void exchange_done(const c_multikeys_PAIR & ID_self,  const c_multikeys_pub & ID_them,
			const std::string & packetstart);

		///! generate and return our full packetstarter
		///!
		string generate_packetstart(c_stream & stream_to_encrypt_with) const;

		///! parse received packetstart and get IDe (the next ID to start next stream)
		string parse_packetstart_kexasym(const string & data) const; ///< parse received packetstart and get kexasym part
		///! parse received packetstart and get IDe (the next ID to start next stream)
		string parse_packetstart_IDe(const string & data) const;

		void set_packetstart_IDe_from(const c_multikeys_PAIR & keypair);

		unique_ptr<c_multikeys_PAIR> create_IDe(bool will_asymkex);

		std::string box(const std::string & msg);
		std::string unbox(const std::string & msg);

		virtual t_crypto_system_type get_system_type() const;

	private:
		t_symkey calculate_KCT(const c_multikeys_PAIR & self,  const c_multikeys_pub & them,
			bool will_new_id, const std::string & packetstart);
		void create_boxer_with_K(); ///< create m_boxer, m_unboxer etc, call this when we have m_KCT set
		t_crypto_system_count get_cryptolists_count_for_KCTf() const;
		static bool calculate_nonce_odd(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);

		static sodiumpp::locked_string return_empty_K();
		bool is_K_not_empty() const; // is K set now
};


/**
 * A CT, can be used to send data in both directions.
*/
class c_crypto_tunnel final {
	private:
		// TODO why 65:
		const size_t m_ntru_dh_random_bytes_size = 65; // max size for NTRU_EES439EP1

		bool m_side_initiator; ///< am I initiator of the dialog (or respondent)

		unique_ptr<c_multikeys_PAIR> m_IDe; ///< our ephemeral ID (that will create KCTf)

		unique_ptr<c_stream> m_stream_crypto_ab; ///< the "ab" crypto - wit KCTab
		unique_ptr<c_stream> m_stream_crypto_final; ///< the ephemeral crypto - with KCTf

	public:
		c_crypto_tunnel(const c_multikeys_PAIR & ID_self, const c_multikeys_pub & ID_them);
		c_crypto_tunnel(const c_multikeys_PAIR & ID_self, const c_multikeys_pub & ID_them,
			const std::string & packetstart );

		void create_IDe();
		void create_CTf(const std::string & packetstart);

		c_multikeys_PAIR & get_IDe(); ///< get our m_IDe needed to create KCTf

		std::string get_packetstart_ab() const;
		std::string get_packetstart_final() const;

		std::string box_ab(const std::string & msg);
		std::string unbox_ab(const std::string & msg);

		std::string box(const std::string & msg);
		std::string unbox(const std::string & msg);

};


// === high level tests ===

void test_crypto();
void test_crypto_benchmark(const size_t seconds_for_test_case);


} // namespace antinet_crypto

// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN


#endif

