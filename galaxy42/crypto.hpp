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
 * * GMK - Galaxy Multi Key - is the name of data format used
 */

using namespace std;

std::string to_debug_locked(const sodiumpp::locked_string & data);


/// @ingroup antinet_crypto
namespace antinet_crypto {


DRBG_HANDLE get_DRBG(size_t size);

// random functions
uint8_t get_entropy(ENTROPY_CMD cmd, uint8_t *out);
/**
 * Generate "nbytes" random bytes and output the result to random_array
 * Returns CRYPTO_SUCCESS (=1) on success, CRYPTO_ERROR (=0) otherwise.
 */
CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array);

namespace unittest {
	class c_symhash_state__tests_with_private_access;
} // namespace


/**
 * \verbatim
 * =====================================================================
 * Above is the crypto at basic level in Galaxy.
 * Types: uses own t_types - that are probably std::string
 * =====================================================================
 * \endverbatim
 */

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
	e_crypto_system_type_END,
};

std::string t_crypto_system_type_to_name(int val);

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


/** State of the SymHash (aka: ratchet?) */
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

/*

class c_multistate {
	typedef std::array<
		vector< unique_ptr< c_crypto_system > > ,
		e_crypto_system_type_END
	> t_cryptolists;

	c_dhdh_state dhdh;

	c_symhash_state symhash;
};

*/

char t_crypto_system_type_to_ID(int val);
t_crypto_system_type t_crypto_system_type_from_ID(char name);

/** All keys, of template type TKey, of given identity. It can be e.g. all public keys, or all private, etc.  */
template <typename TKey>
class c_multikeys_general : public c_crypto_system {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multikeys_general<TKey> t_self; ///< My own type (concretized ofcourse). Useful for shorter coding style.

		typedef std::array< vector< TKey > , e_crypto_system_type_END	> t_cryptolists_general; ///< templated!
		typedef TKey t_key; ///< concretized key type used in this templated class

		///< A "map" of [e.g.] public keys of given type, organized by their crypto type.
		///< example use: to get 50-th of our Ed25519 keys: m_cryptolists_pubkey[ e_crypto_system_type_Ed25519 ].at(50);
		t_cryptolists_general m_cryptolists_general; ///<  *** The main collection ("map") of the keys

		mutable string m_hash_cached; ///< Hash of all my [e.g.] public keys (a cache,
		// auto calculated by getters/cleared by setters - when m_autoupdate_hash)
		// empty "" means that it needs calculation. (this is the default value)

		void update_hash() const; ///< calculate the current m_hash and save it

	protected:
		/// @name Modifiers - general version. \n(that sould be wrapped in child class) @{
		void add_key(t_crypto_system_type crypto_type, const TKey & key); ///< append one more key
		/// @}

		/// @name Getters - general version. \n(that sould be wrapped in child class) @{
		TKey get_key(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}

	public:
		c_multikeys_general()=default;
		virtual ~c_multikeys_general()=default;

		/// @name Getters: @{
		size_t get_count_keys_in_system(t_crypto_system_type crypto_type) const; ///< how many keys of given type
		size_t get_count_of_systems() const; ///< how many key types?

		string get_hash() const; ///< const, though it is allowed to update mutable field with cache of current hash
		/// @}

		/// @name save/load: @{
		virtual std::string serialize_bin() const; ///< returns a string with all our data serialized, see load_from_bin() for details

		virtual void load_from_bin(const std::string & data); ///< set this object to data loaded from string from serialize_bin().
		///< Will delete any prior data in this object.
		///< Will be always compatible with older/newer versions (across stable releases of the program)
		///< @warning Must remain compatible especially because this can change the resulting HIP address!

		void save(const string  & fname) const; ///< Save this data to a file.
		void load(const string  & fname); ///< Replace all current data with data loaded from this file.
		void clear(); ///< Delete all current data.
		/// @}

		bool operator>(const t_self &rhs) const; ///< some sorting order, e.g. using the get_hash(). Used e.g. to
		/// pick even/odd nonce depending on comparing keys.

};

/** All pubkeys of given identity */
class c_multikeys_pub : public c_multikeys_general<c_crypto_system::t_pubkey> {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multikeys_general<t_pubkey>::t_cryptolists_general  t_cryptolists_pubkey;

	public:
		virtual t_crypto_system_type get_system_type() const;

		/// @name Modifiers - concretized version. \n Ready to use. @{
		void add_public(t_crypto_system_type crypto_type, const t_key & key); ///< append one more key
		/// @}

		/// @name Getters - concretized version. \n Ready to use. @{
		t_key get_public(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}
};

/** All PRIVATE keys of given identity */
class c_multikeys_PRV : public c_multikeys_general<c_crypto_system::t_PRVkey> {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef c_multikeys_general<t_PRVkey>::t_cryptolists_general  t_cryptolists_PRVkey;

	public:
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

		void generate(t_crypto_system_type crypto_system_type, int count=1); ///< generate and save e.g. 3 X25519 keys
		void generate(); ///< generate the default set of keys
		static std::pair<sodiumpp::locked_string, std::string> generate_x25519_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_nrtu_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_sidh_key_pair();

		void debug() const;

		void add_public_and_PRIVATE(t_crypto_system_type crypto_type,
		const c_crypto_system::t_pubkey & pubkey ,
		const c_crypto_system::t_PRVkey & PRVkey);

		void save_PRV(const string  & fname_base) const; ///< like c_multikeys_general<>::save(), for the private (+public) key
		void save_pub(const string  & fname_base) const; ///< like c_multikeys_general<>::save(), for the public key
		void load_PRV(const string  & fname_base); ///< like c_multikeys_general<>::load(), for the private (+public) key

		virtual t_crypto_system_type get_system_type() const;
};


/**
 * Crypto primitives are provided by - sodiumpp library (and NTru, SIDH, Geport - in future - TODO)
 * Raw symmetric encryption - done by c_stream_crypto
 * Adding needed crypto data - nonce constant, nonce counter - done by c_crypto_tunnel
 * Downloading pubkeys, adding other meta-data, transport - are to be done by other, higher layers.
 */


/**
 * The KCT crypto system of the stream, ready for finall use.
 * It does only the main crypto algorithm.
 */
class c_stream_crypto final /* because strange ctor init list functions */
: public c_crypto_system
{
	private:
		const size_t m_ntru_dh_random_bytes_size = 2;
		sodiumpp::locked_string m_ntru_dh_random_bytes;
		std::vector<std::string> m_ntru_ecrypt_to_them_rand; ///< m_ntru_dh_random_bytes encrypted by all ntru pub keys
		t_symkey m_KCT; ///< the KCT for this stream
		bool m_nonce_odd; ///< is our key uneven (odd) as used in sodiumpp to decide nonce for us
		// TODO lock it's memory before setting it!!!

		sodiumpp::boxer< t_crypto_nonce > m_boxer;
		sodiumpp::unboxer< t_crypto_nonce > m_unboxer;

	public:
		virtual t_crypto_system_type get_system_type() const;

		c_stream_crypto(const c_multikeys_PAIR & IDC_self,  const c_multikeys_pub & IDC_them);
		c_stream_crypto(const c_multikeys_PAIR & IDC_self,  const c_multikeys_pub & IDC_them, std::vector<std::string> ntru_rand_encrypt_to_me);

		std::string box(const std::string & msg);
		std::string unbox(const std::string & msg);
		std::vector<std::string> get_ntru_encrypt_rand();

	private:
		t_symkey calculate_KCT(const c_multikeys_PAIR & self,  const c_multikeys_pub & them, std::vector<std::string> ntru_rand_encrypt_to_me);
		static bool calculate_nonce_odd(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);

};

/** A CT, can be used to send data in both directions. */
class c_crypto_tunnel final {
	private:
		unique_ptr<c_stream_crypto> m_stream_crypto_ab; ///< the "ab" crypto - wit KCTab
		unique_ptr<c_stream_crypto> m_stream_crypto_final; ///< the ephemeral crypto - with KCTf

	public:
		c_crypto_tunnel()=default;
		//c_crypto_tunnel(const c_multikeys_PAIR & IDC_self,  const c_multikeys_pub & IDC_them);
		c_crypto_tunnel(const c_multikeys_PAIR & IDC_self,  const c_multikeys_pub & IDC_them, std::vector<std::string> them_encrypted_ntru_rand);

		std::string box(const std::string & msg);
		std::string unbox(const std::string & msg);
		std::vector<std::string> get_encrypt_ntru_rand();
};

c_crypto_tunnel create_crypto_tunnel(c_multikeys_PAIR & self, c_multikeys_pub & other);

#if 0


/**
 * For given CT (form me, to given recipient) - and given session
 * TODO this is not really used (instead see c_stream_crypto)
 */
class c_dhdh_state final : public c_crypto_system {
	public:
		typedef string_as_bin t_pubkey;
		typedef string_as_bin t_PRVkey;
		typedef long long int t_nonce;

		/// shared key from permanent
		/// KP = complete_DH( ap , BP ) , ()
		/// prepare permanent-key based AuthEncr
		t_symkey m_skp;
		c_dhdh_state(t_PRVkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub);

		void step1();
		t_pubkey get_permanent_pubkey() const;
		t_pubkey get_temp_pubkey() const;
		static std::pair<t_pubkey, t_PRVkey> generate_key_pair();

	private:
		t_PRVkey m_our_priv; ///< permanent
		t_pubkey m_our_pub; ///< permanent
		t_pubkey m_theirs_pub;
		t_symkey m_r; ///< my random r1 or r2 to use in CA shared key // TODO(r) not used...

		t_pubkey m_pubkey_temp;
		t_PRVkey m_privkey_temp;

		t_symkey execute_DH_exchange(const t_PRVkey &my_priv, const t_pubkey &my_pub, const t_pubkey &theirs_pub);
		t_symkey execute_DH_exchange(); ///< call execute_DH_exchange with internal fields

		FRIEND_TEST(crypto, dh_exchange);

		t_nonce m_nonce;
};

#endif

void test_crypto();
void test_crypto_benchmark(const size_t seconds_for_test_case);


} // namespace


#endif

