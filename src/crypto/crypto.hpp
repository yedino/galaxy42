// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

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




#pragma once
#ifndef include_crypto_hpp
#define include_crypto_hpp

#include "../libs1.hpp"
#include <sodium.h>
#include "../strings_utils.hpp"
#include "gtest/gtest_prod.h"
#include <sodiumpp/sodiumpp.h>

#if ENABLE_CRYPTO_NTRU
	#ifdef __MACH__
		#include "ntru/ntru_crypto_drbg.h"
		#include "ntru/ntru_crypto.h"
	#else
		#include "ntru/include/ntru_crypto_drbg.h"
		#include "ntru/include/ntru_crypto.h"
	#endif
#endif

#if ENABLE_CRYPTO_SIDH
	#include <SIDH.h>
#endif

#include "crypto_basic.hpp"
#include "multikeys.hpp"
#include "httpdbg/httpdbg-server.hpp"

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

------

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

------
                                                                  Hello - datagram (e.g. UDP)
                                                             1052 Hello - packet (e.g. UDP)

                                           fd42::01 fd42::02 1052 Hello - tundata - cleartext of user's tunneled data
                                           7bqehbu8hbqhoba89hrgeg8qh8gq - blob - encrypted (end2end) user's tunneled data

                   TTL=5  DST_HIP=fd42:002 7bqehbu8hbqhoba89hrgeg8qh8gq - cmddata

V=1 CMD=tunneldata TTL=5 DST_HIP=fd42:002 7bqehbu8hbqhoba89hrgeg8qh8gq [AUTH-P2P] - protomsg (end2end encrypted)

 */

/// @ingroup antinet_crypto
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace antinet_crypto {


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
    friend class ::c_httpdbg_raport;
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
		c_stream(bool side_initiator, const string& nicename);
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
		std::string box(const std::string & msg, t_crypto_nonce & nonce); ///< box this cleartext, and OUT the nonce that was used
		std::string unbox(const std::string & msg);
		std::string unbox(const std::string & msg, t_crypto_nonce nonce, bool force_nonce=1); ///< unbox, but using given nonce

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
class c_crypto_tunnel {
    friend class ::c_httpdbg_raport;
	private:
		// TODO why 65:
		const size_t m_ntru_dh_random_bytes_size = 65; // max size for NTRU_EES439EP1

		bool m_side_initiator; ///< am I initiator of the dialog (or respondent)

		unique_ptr<c_multikeys_PAIR> m_IDe; ///< our ephemeral ID (that will create KCTf)

		unique_ptr<c_stream> m_stream_crypto_ab; ///< the "ab" crypto - wit KCTab
		unique_ptr<c_stream> m_stream_crypto_final; ///< the ephemeral crypto - with KCTf

		string m_nicename; ///< my nice name for logging/debugging

	public:
		c_crypto_tunnel(const c_multikeys_PAIR & ID_self, const c_multikeys_pub & ID_them, const string& nicename);
		c_crypto_tunnel(const c_multikeys_PAIR & ID_self, const c_multikeys_pub & ID_them,
			const std::string & packetstart, const string& nicename );

		virtual ~c_crypto_tunnel()=default;

		std::string debug_this() const;

		void create_IDe();
		void create_CTf(const std::string & packetstart);

		c_multikeys_PAIR & get_IDe(); ///< get our m_IDe needed to create KCTf

		std::string get_packetstart_ab() const;
		std::string get_packetstart_final() const;

		std::string box_ab(const std::string & msg);
		std::string box_ab(const std::string & msg, t_crypto_nonce & nonce); ///< box this cleartext, and OUT the nonce that was used
		std::string unbox_ab(const std::string & msg);
		std::string unbox_ab(const std::string & msg, t_crypto_nonce nonce); ///< unbox, but using given nonce

		std::string box(const std::string & msg);
		std::string box(const std::string & msg, t_crypto_nonce & nonce); ///< box this cleartext, and OUT the nonce that was used
		std::string unbox(const std::string & msg);
		std::string unbox(const std::string & msg, t_crypto_nonce nonce); ///< unbox, but using given nonce

};


// === high level tests ===

void test_crypto();
void generate_keypairs_benchmark(const size_t seconds_for_test_case);
void stream_encrypt_benchmark(const size_t seconds_for_test_case);
void multi_key_sign_generation_benchmark(const size_t seconds_for_test_case);


} // namespace antinet_crypto

// NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN


#endif

