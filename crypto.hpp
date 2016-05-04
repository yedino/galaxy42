#ifndef include_crypto_hpp
#define include_crypto_hpp

#include "libs1.hpp"
#include <sodium.h>
#include "strings_utils.hpp"
#include "gtest/gtest_prod.h"

#include <sodiumpp/sodiumpp.h>


using namespace std;

namespace antinet_crypto {

namespace unittest {
	class c_symhash_state__tests_with_private_access;
} // namespace


/***
=====================================================================
Above is the crypto at basic level in Galaxy.
Types: uses own t_types - that are probably std::string
=====================================================================
*/

// must match: t_crypto_system_type_to_name() 
enum t_crypto_system_type : unsigned char {
	// 0 is reserved
	e_crypto_system_type_invalid = 0,
	e_crypto_system_type_X25519 = 1,
	e_crypto_system_type_Ed25519 = 2,
	e_crypto_system_type_ntru128 = 3,
	e_crypto_system_type_geport_todo = 4,
	e_crypto_system_type_symhash_todo = 5,
	e_crypto_system_type_END_normal,

	e_crypto_system_type_multikey_pub = 6,
	e_crypto_system_type_multikey_private = 6,
	e_crypto_system_type_END,
};

/*** Some state of some crypto system */
class c_crypto_system {
	public:
		typedef std::string t_symkey; //< type of symmetric key
		typedef std::string t_hash; //< type of hash
		typedef std::string t_pubkey; //< type of public key
		typedef std::string t_PRIVkey; //< type of private key

		typedef sodiumpp::nonce64 t_crypto_nonce; //< the type of nonce that we use

		virtual ~c_crypto_system()=default;

		virtual t_symkey secure_random(size_t size_of_radom_data) const;

		virtual t_hash Hash1( const t_hash & hash ) const;
		virtual t_hash Hash2( const t_hash & hash ) const;

		virtual t_crypto_system_type get_system_type() const;
};


/*** State of the SymHash (aka: ratchet?) */
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


/*** All pubkeys of given identity */
class c_multikeys_pub : c_crypto_system {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef std::array< vector< t_pubkey > , e_crypto_system_type_END	> t_cryptolists_pubkey;
		t_cryptolists_pubkey m_cryptolists_pubkey; //< A "map" of public keys of given type, organized by their crypto type.
		//< example use: to get 50-th of our Ed25519 keys: m_cryptolists_pubkey[ e_crypto_system_type_Ed25519 ].at(50);

	public:
		void add_public(t_crypto_system_type crypto_type, const t_pubkey & pubkey);
		virtual t_crypto_system_type get_system_type() const;
};

/*** All PRIVATE keys of given identity */
class c_multikeys_PRIV : c_crypto_system {
	protected:
		friend class c_multikeys_PAIR;
		friend class c_crypto_system;

		typedef std::array< vector< t_PRIVkey > , e_crypto_system_type_END	> t_cryptolists_PRIVkey;
		t_cryptolists_PRIVkey m_cryptolists_PRIVkey; //< A "map" of PRIVATE keys of given type, organized by their crypto type.
		//< example use: to get 50-th of our Ed25519 keys: m_cryptolists_PRIVkey[ e_crypto_system_type_Ed25519 ].at(50);

	public:
		void add_PRIVATE(t_crypto_system_type crypto_type,const t_PRIVkey & PRIVkey);
		virtual t_crypto_system_type get_system_type() const;
};

class c_multikeys_PAIR {
	public:
		c_multikeys_pub m_pub;
		c_multikeys_PRIV m_PRIV;

		void generate();

		void debug() const;

		void add_public_and_PRIVATE(t_crypto_system_type crypto_type,
			const c_crypto_system::t_pubkey & pubkey ,
			const c_crypto_system::t_PRIVkey & PRIVkey);

		virtual t_crypto_system_type get_system_type() const;
};


/*** The finall crypto of the stream */
class c_stream_crypto final /* because strange ctor init list functions */ 
: public c_crypto_system 
{
	protected:
		t_symkey m_usable_key; //< UK (a.k.a. "K") is the finall key to be used on the streams
		bool m_nonce_odd; //< is our key uneven (odd) as used in sodiumpp to decide nonce for us
		// TODO lock it's memory before setting it!!!

		sodiumpp::boxer< t_crypto_nonce > m_boxer;
		sodiumpp::unboxer< t_crypto_nonce > m_unboxer;

	public:
		virtual t_crypto_system_type get_system_type() const;

		c_stream_crypto(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);
	
	private:
		static t_symkey calculate_usable_key(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);
		static bool calculate_nonce_odd(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);

};

/*** Tool ready to process the stream as CryptoTunnel (like CryptoAuth from Cjdns naming) */
class c_crypto_tunnel {
	protected:
		unique_ptr<c_stream_crypto> m_stream_crypto;

	public:
		c_crypto_tunnel()=default;
		c_crypto_tunnel(const c_multikeys_PAIR & self,  const c_multikeys_pub & them);
};

c_crypto_tunnel create_crypto_tunnel(c_multikeys_PAIR & self, c_multikeys_pub & other);


/** For given CA (form me, to given recipient) - and given session */
class c_dhdh_state final : public c_crypto_system {
	public:
		typedef string_as_bin t_pubkey;
		typedef string_as_bin t_PRIVkey;
		typedef long long int t_nonce;

		t_symkey m_skp; // shared key from permanent // KP = complete_DH( ap , BP ) , () // prepare permanent-key based AuthEncr

		c_dhdh_state(t_PRIVkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub);

		void step1();
		t_pubkey get_permanent_pubkey() const;
		t_pubkey get_temp_pubkey() const;
		static std::pair<t_pubkey, t_PRIVkey> generate_key_pair();

	private:
		t_PRIVkey m_our_priv; ///< permanent
		t_pubkey m_our_pub; ///< permanent
		t_pubkey m_theirs_pub;
		t_symkey m_r; // my random r1 or r2 to use in CA shared key

		t_pubkey m_pubkey_temp;
		t_PRIVkey m_privkey_temp;

		t_symkey execute_DH_exchange(const t_PRIVkey &my_priv, const t_pubkey &my_pub, const t_pubkey &theirs_pub);
		t_symkey execute_DH_exchange(); ///< call execute_DH_exchange with internal fields

		FRIEND_TEST(crypto, dh_exchange);

		t_nonce m_nonce;
};



void test_crypto();


} // namespace


#endif

