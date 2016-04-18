#ifndef include_crypto_hpp
#define include_crypto_hpp

#include "libs1.hpp"
#include <sodium.h>
#include "strings_utils.hpp"
#include "gtest/gtest_prod.h"

using namespace std;

namespace antinet_crypto {

namespace unittest {
class c_symhash_state__tests_with_private_access;
} // namespace



/***
 * Some state of some crypto system
*/
class c_crypto_state {
	public:

		typedef string_as_bin t_symkey; // symmetric key

		virtual ~c_crypto_state()=default;

		virtual t_symkey secure_random(size_t size_of_radom_data) const;
};



class c_symhash_state final : public c_crypto_state {
	public:
		typedef string_as_bin t_hash;

		c_symhash_state( t_hash initial_state );
		void next_state( t_hash additional_secret_material = t_hash("") );
		t_hash get_password() const;

		t_hash Hash1( const t_hash & hash ) const;
		t_hash Hash2( const t_hash & hash ) const;

	private:
		t_hash get_the_SECRET_PRIVATE_state() const; //< used e.g. by some tests
		friend class unittest::c_symhash_state__tests_with_private_access;
		FRIEND_TEST(crypto, aeshash_not_repeating_state_nor_password);

	
	private:
		t_hash m_state;
		int m_number; ///< for debug mostly for now
};




// ... 
// For given CA (form me, to given recipient) - and should be permanent

// ==================================================================

/***

DHDH CryptoAuth


On ID creation:
Ali: ap,AP = Generte_DH // make permanent keys: ap = Ali_DH_Perm_priv , AT = Ali_DH_Perm_Pub

On ID creation:
Bob: bp,BP = Generte_DH // same for Bob


On CA establishing from Alice to Bob:
Alice: 
  at,AT = Generte_DH()  // make temporary keys: at = Ali_DH_Tmp_priv , AT = Ali_DH_Tmp_Pub
  r1 = credentials_given_by_b || ';' || secure_random
  mt = "AT,r1"; // message with temporary key and temporary random

  KP = complete_DH( ap , BP ) , () // prepare permanent-key based AuthEncr
  uap=secure_random() ; ua = uap || encrypt_symm( KP , uap || mt )  // authenticate temp message
  deltaua = encrypt_symm ( HASHED_SYM_STATE.get_password() , uap )  // anti-QC!

Ali -> Bob: "deltaua" // NETWORK send

Bob: 
  decrypts/checks auth: ua 
  has AT, r1

  bt,BT = Generte_DH()  // make temporary keys: at = Ali_DH_Tmp_priv , AT = Ali_DH_Tmp_Pub
  r2 = secure_random() 
  mt = "BT,r2"; // message with temporary key and temporary random

  KP = complete_DH( bp , AP ) , // prepare permanent-key AuthEncr  
  ubp=secure_random() ; ub = ubp || encrypt_symm( KP , ubp || mt )  // authenticate temp message
  ... or ub = encrypt_symm( KP , ubp || ubp || mt )
  ... or ub = encrypt_symm( KP , 'all_is_fine' || mt )
  deltaub = encrypt_symm ( HASHED_SYM_STATE.get_password() , uab )  // anti-QC!

Bob -> Alice: "deltaub" // NETWORK send

Alice (and Bob similar)
  decrypts/checks auth: ub (and decrypts with HASHED_SYM_STATE)
  has BT, r2

Now both sides calculate the same:
  TSK = complete_DH( at , BT)  or  = complete_DH( bt , AT) // Temporary Shared Key
  FSSSK = H( H( TSK ) || H( r1 ) || H( r2 ) )   // Forward-Secrecy Session Shared Key
  nonce=1

*** r1,r2 help MAYBE(?) when:
attack hardness:   badluck-DH << DH-tmp , DH-perm 

*/


// For given CA (form me, to given recipient) - and given session
class c_dhdh_state final : public c_crypto_state {
	public:
		typedef string_as_bin t_pubkey;
		typedef string_as_bin t_privkey;
		typedef long long int t_nonce;

		t_symkey m_skp; // shared key from permanent // KP = complete_DH( ap , BP ) , () // prepare permanent-key based AuthEncr

		c_dhdh_state(t_privkey our_priv, t_pubkey our_pub, t_pubkey theirs_pub);

		void step1();
		t_pubkey get_permanent_pubkey() const;
		t_pubkey get_temp_pubkey() const;

	private: 
		t_privkey m_our_priv; ///< permanent
		t_pubkey m_our_pub; ///< permanent
		t_pubkey m_theirs_pub;
		t_symkey m_r; // my random r1 or r2 to use in CA shared key

		t_pubkey m_pubkey_temp;
		t_privkey m_privkey_temp;

		t_symkey execute_DH_exchange(const t_privkey &my_priv, const t_pubkey &my_pub, const t_pubkey &theirs_pub);
		void generate_temp_key_pair(); ///< save generated keys to m_pubkey_temp and m_privkey_temp


		t_nonce m_nonce;
};




void test_crypto();
	

} // namespace


#endif

