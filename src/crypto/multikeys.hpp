// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef include_crypto_multikeys_hpp
#define include_crypto_multikeys_hpp


#include "../libs1.hpp"
#include <sodium.h>
#include "../strings_utils.hpp"
#include "gtest/gtest_prod.h"
#include <sodiumpp/sodiumpp.h>
#include <SIDH.h>

#include "crypto_basic.hpp"

#ifdef __MACH__
#include "ntru/ntru_crypto_drbg.h"
#else
#include "ntru/include/ntru_crypto_drbg.h"
#endif

namespace antinet_crypto {


// ##################################################################
// ##################################################################
// all about the multikeys

// ==================================================================


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
		void set_hash_dirty();

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
		c_multisign ();
		virtual t_crypto_system_type get_system_type() const;

		void add_signature_vec (const std::vector<std::string> &signatures_vec,
							   t_crypto_system_type crypto_type);
		void add_signature (t_crypto_system_type crypto_type, const std::string &signature);

		std::vector<std::string> get_signature_vec (t_crypto_system_type crypto_type) const;
		std::string get_signature (t_crypto_system_type crypto_type, size_t number_of_key) const;

		/**
		 * @brief print_signatures debug function
		 * 						   that print all signatures
		 */
		void print_signatures () const;
		/**
		 * @brief cryptosystem_sign_allowed
		 * @param crypto_system
		 * @return true if crypto system have ability to signing messages
		 * 		   false otherwise
		 */
		static bool cryptosystem_sign_allowed (t_crypto_system_type crypto_system);

		std::string serialize_bin () const override;
		void load_from_bin (const std::string & data) override;
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

		string get_ipv6_string_bin() const; ///< IPV6 from hash of this key - as binary data string
		string get_ipv6_string_hex() const; ///< IPV6 from hash of this key - as hex values string
		string get_ipv6_string_hexdot() const; ///< IPV6 from hash of this key in ipv6 format hex + dots

		/// @name Modifiers - concretized version. \n Ready to use. @{
		void add_public(t_crypto_system_type crypto_type, const t_key & key); ///< append one more key
		/// @}

		/// @name Getters - concretized version. \n Ready to use. @{
		t_key get_public(t_crypto_system_type crypto_type, size_t number_of_key) const;
		/// @}

		/// @{
		/**
		 * @brief multi_sign_verify	Verify message that was signed multiple by c_multikeys_pub.
		 * 		  Throw sodiumpp::crypto_error when verify fails.
		 * @param msg Message that was signed.
		 * @param signs	Vector of signs
		 * @param pubkeys c_multikeys_pub that contain according to signs vector of public keys.
		 * 		  Public keys must coresponding to PRV keys that was used to sign message
		 * @param sign_type	Type of crypto system.
		 */
		static void multi_sign_verify(const std::vector<string> &signs,
									  const string &msg,
									  const c_multikeys_pub &pubkeys,
									  t_crypto_system_type sign_type);

		static void multi_sign_verify(const c_multisign &all_signatures,
									  const std::string &msg,
									  const c_multikeys_pub &pubkeys);
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

		/// @{
		/**
		 * @brief multi_sign Multiple signing message using all keys of given sign type
		 * @param msg Massage that will be sign
		 * @param sign_type	Type of crypto system
		 * @return Vector of signs
		 */
		c_multisign multi_sign(const std::string &msg);

		std::vector<std::string> multi_sign(const std::string &msg,
											t_crypto_system_type sign_type);
		/// @}
};




class c_multikeys_PAIR {
	public:
		// TODO move to private:
		c_multikeys_pub m_pub;
		c_multikeys_PRV m_PRV;

	public:
		virtual ~c_multikeys_PAIR() = default;

		string get_ipv6_string_bin() const; ///< IPV6 from hash of this key (binary data)
		string get_ipv6_string_hex() const; ///< IPV6 from hash of this key (hex string)
		string get_ipv6_string_hexdot() const; ///< IPV6 from hash of this key in ipv6 format hex + dots

		string get_serialize_bin_pubkey() const; ///< get the serialized pubkey of this, e.g. to send it to others

		///< generate from list of how many keys of given type we need
		void generate(t_crypto_system_count cryptolists_count, bool will_asymkex);
		void generate(t_crypto_system_type crypto_system_type, int count=1); ///< generate and save e.g. 3 X25519 keys
		void generate(); ///< generate the default set of keys
		static std::pair<sodiumpp::locked_string, std::string> generate_x25519_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_ed25519_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_nrtu_encrypt_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_nrtu_sign_key_pair();
		static std::pair<sodiumpp::locked_string, std::string> generate_sidh_key_pair();

		/// @{
		/**
		 * @brief multi_sign Multiple signing message using all keys of given sign type
		 * @param msg Massage that will be sign
		 * @param sign_type	Type of crypto system
		 * @return Vector of signs
		 */
		c_multisign multi_sign(const std::string &msg);

		std::vector<std::string> multi_sign(const std::string &msg,
											t_crypto_system_type sign_type);
		/// @}
		/// @{
		/**
		 * @brief multi_sign_verify	Verify message that was signed multiple by c_multikeys_pub.
		 * 		  Throw sodiumpp::crypto_error when verify fails.
		 * @param msg Message that was signed.
		 * @param signs	Vector of signs
		 * @param pubkeys c_multikeys_pub that contain according to signs vector of public keys.
		 * 		  Public keys must coresponding to PRV keys that was used to sign message
		 * @param sign_type	Type of crypto system.
		 */
		void multi_sign_verify(const std::vector<string> &signs,
									  const string &msg,
									  t_crypto_system_type sign_type);

		void multi_sign_verify(const c_multisign &all_signatures,
									  const std::string &msg);
		/// @}

		void debug() const;

		void add_public_and_PRIVATE(t_crypto_system_type crypto_type,
			const c_crypto_system::t_pubkey & pubkey,
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






} // namespace


#endif






