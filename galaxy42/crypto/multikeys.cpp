
#include "multikeys.hpp"

#include "crypto.hpp"

#include "../crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include "../build_extra/ntru/include/ntru_crypto.h"
#include <SIDH_internal.h>

#include "../trivialserialize.hpp"

#include "../filestorage.hpp"
#include "../text_ui.hpp"
#include "ntrupp.hpp"

#include "../glue_lockedstring_trivialserialize.hpp"
#include "../glue_sodiumpp_crypto.hpp"

#include "multikeys.tpl.hpp"

namespace antinet_crypto {

using sodiumpp::locked_string;


t_crypto_system_type c_multikeys_pub::get_system_type() const { return e_crypto_system_type_multikey_pub; }

t_crypto_system_type c_multikeys_PRV::get_system_type() const { return e_crypto_system_type_multikey_private; }

t_crypto_system_type c_multikeys_PAIR::get_system_type() const { return e_crypto_system_type_multikey_private; }






// TODONOW remove?
void instantiate() {
	{ c_multicryptostrings<std::string> someobject(e_crypto_use_open); }
	{ c_multicryptostrings<sodiumpp::locked_string> someobject(e_crypto_use_open); }
	{ c_multikeys_general<std::string> someobject(e_crypto_use_open); }
	{ c_multikeys_general<sodiumpp::locked_string> someobject(e_crypto_use_open); }
}

// ==================================================================
// c_multisign

c_multisign::c_multisign() : c_multicryptostrings(e_crypto_use_signature)
{ }

t_crypto_system_type c_multisign::get_system_type() const { return e_crypto_system_type_multisign; }

void c_multisign::add_signature_vec(const std::vector<string> &signatures_vec, t_crypto_system_type crypto_type) {
	//std::cout << "adding vec " << t_crypto_system_type_to_name(crypto_type)	// dbg
	//		  << " vec_size: " << signatures_vec.size() << std::endl;
	for(size_t i = 0; i < signatures_vec.size(); ++i) {
		add_signature(crypto_type, signatures_vec[i]);
	}
}

void c_multisign::add_signature(t_crypto_system_type crypto_type, const string &signature) {
	//std::cout << "adding key " << to_debug_locked_maybe(signature) << std::endl;	// dbg
	add_key(crypto_type, signature);
}

std::vector<string> c_multisign::get_signature_vec(t_crypto_system_type crypto_type) const {
	std::vector<std::string> signatures_vec;
	for (size_t i = 0; i < get_count_keys_in_system(crypto_type); ++i) {
		signatures_vec.emplace_back(std::move(get_signature(crypto_type, i)));
	}
	return signatures_vec;
}

string c_multisign::get_signature(t_crypto_system_type crypto_type, size_t number_of_key) const {
	return get_key(crypto_type, number_of_key);
}

void c_multisign::print_signatures() const {
	_info("Printing all sinatures in c_multisign:");
	for (size_t sys = 0; sys < get_count_of_systems(); ++sys) {
		auto sys_enum = int_to_enum<t_crypto_system_type>(sys);
		for (size_t i = 0; i < get_count_keys_in_system(sys_enum); ++i) {
			_info("[" << i << "]."
				  << t_crypto_system_type_to_name(sys_enum)
				  << " signature: " << to_debug_locked_maybe(get_key(sys_enum, i)));
		}
	}
}

bool c_multisign::cryptosystem_sign_allowed(t_crypto_system_type crypto_system) {
	// if list all crypto systems allowed for signing

	if (crypto_system == e_crypto_system_type_Ed25519 ||
			crypto_system == e_crypto_system_type_NTRU_sign) {
		return true;
	}
	return false;
}

// ==================================================================
// c_multikeys_pub

c_multikeys_pub::c_multikeys_pub()
	: c_multikeys_general<c_crypto_system::t_pubkey>( e_crypto_use_open )
{ }

string c_multikeys_pub::get_ipv6_string() const {
	string hash = get_hash();
	string prefix = "fd48";
	// still discussion how to generate address regarding number of bruteforce cost to it TODO

	const int len_ip = 128/8; // needed for ipv6
	const int len_pre = prefix.size();

	string ip = prefix + hash.substr(0, len_ip - len_pre);
	return ip;
}

void c_multikeys_pub::add_public(t_crypto_system_type crypto_type, const t_key & key) {
	add_key(crypto_type, key);
}

c_multikeys_pub::t_key c_multikeys_pub::get_public(t_crypto_system_type crypto_type, size_t number_of_key) const {
	return get_key(crypto_type, number_of_key);
}



// ==================================================================
// c_multikeys_PRV


c_multikeys_PRV::c_multikeys_PRV()
	: c_multikeys_general<c_crypto_system::t_PRVkey>( e_crypto_use_secret )
{ }

void c_multikeys_PRV::add_PRIVATE(t_crypto_system_type crypto_type, const t_key & key) {
	add_key(crypto_type, key);
}

c_multikeys_PRV::t_key c_multikeys_PRV::get_PRIVATE(t_crypto_system_type crypto_type, size_t number_of_key) const {
	return get_key(crypto_type, number_of_key);
}



// ==================================================================
// c_multikeys_PAIR


string c_multikeys_PAIR::get_ipv6_string() const {
	return m_pub.get_ipv6_string();
}

void c_multikeys_PAIR::debug() const {
	_info("KEY PAIR:");

	for (size_t ix=0; ix<m_pub.m_cryptolists_general.size(); ++ix) { // for all key type (for each element)
		const auto & pubkeys_of_this_system  = m_pub.m_cryptolists_general.at(ix); // take vector of keys
		const auto & PRVkeys_of_this_system  = m_PRV.m_cryptolists_general.at(ix); // take vector of keys
		const auto size_pub = pubkeys_of_this_system.size();
		const auto size_PRV = PRVkeys_of_this_system.size();
		if (size_pub || size_PRV) {
			if (size_pub != size_PRV) _warn("Not matching count of pub and prv keys! " << size_pub << " vs " << size_PRV);
			for (size_t key_nr=0; key_nr<std::min(size_pub , size_PRV); ++key_nr) {
				const auto name = t_crypto_system_type_to_name(ix);
				_info("pub " + name + " #" << key_nr << "/" << size_pub << " "
					<< to_debug( pubkeys_of_this_system.at(key_nr)) );
				_info("PRV " + name + " #" << key_nr << "/" << size_PRV << " "
					<< to_debug_locked( PRVkeys_of_this_system.at(key_nr) ));
			}
		}
	}
	_info("---------");
}

void c_multikeys_PAIR::datastore_save_PRV_and_pub(const string  & fname_base) const {
	m_PRV.datastore_save(fname_base);
	m_pub.datastore_save(fname_base);
}
void c_multikeys_PAIR::datastore_save_pub(const string  & fname_base) const {
	m_pub.datastore_save(fname_base);
}
void c_multikeys_PAIR::datastore_load_PRV_and_pub(const string  & fname_base) {
	m_PRV.datastore_load(fname_base);
	m_pub.datastore_load(fname_base);
}

c_multikeys_PAIR & c_crypto_tunnel::get_IDe() {
	return * PTR(m_IDe);
}

void c_multikeys_PAIR::generate(t_crypto_system_count cryptolists_count, bool will_asymkex) {
	_info("Generating from cryptolists_count");
	for (size_t sys=0; sys<cryptolists_count.size(); ++sys) { // all key crypto systems
		// for given crypto system:
		auto sys_id = int_to_enum<t_crypto_system_type>(sys); // ID of this crypto system
		auto how_many = cryptolists_count.at(sys);
		if (how_many > 0) {
			_info("Generate keys " << t_crypto_system_type_to_name(sys_id) << " in amount: "<<how_many);
			if ( will_asymkex || false==t_crypto_system_type_is_asymkex(sys_id) ) {
				this->generate(sys_id, how_many);
			} else _dbg1("Skipping because this is asymkex, and we do not want to do that "
				<< "will_asymkex="<<will_asymkex<<" : " << to_string(sys_id));
		}
	}
}

void c_multikeys_PAIR::generate() {
	_info("generate X25519");
	generate( e_crypto_system_type_X25519 , 1 );
	_info("generate NTRU");
	generate( e_crypto_system_type_NTRU_EES439EP1 , 1 );
	_info("generate SIDH");
	generate( e_crypto_system_type_SIDH , 1 );
}

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_x25519_key_pair() {
	size_t s = crypto_scalarmult_SCALARBYTES;
	sodiumpp::randombytes_locked(s);
	auto rnd = sodiumpp::randombytes_locked(s);
	//_info("Random data size=" << (rnd.size()) );
	//_info("Random data=" << to_debug_locked(rnd) );
	sodiumpp::locked_string key_PRV(rnd); // random secret key
	std::string key_pub( sodiumpp::generate_pubkey_from_privkey(key_PRV) ); // PRV -> pub
	return std::make_pair(std::move(key_PRV), std::move(key_pub));
}

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_ed25519_key_pair() {
	std::string public_key;
	size_t privkey_size = crypto_sign_SECRETKEYBYTES;
	sodiumpp::locked_string private_key(privkey_size);
	public_key = sodiumpp::crypto_sign_keypair(private_key);

	return std::make_pair(std::move(private_key), std::move(public_key));
}

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_nrtu_encrypt_key_pair() {
	// generate key pair
	auto keypair = ntrupp::generate_encrypt_keypair();
	// values for NTRU_EES439EP1
	assert(keypair.first.size() == 659);
	assert(keypair.second.size() == 609);

	return keypair;
}

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_nrtu_sign_key_pair() {

	auto keypair = ntrupp::generate_sign_keypair();
	assert(keypair.first.size() == 6152);
	assert(keypair.second.size() == 6158);
	return keypair;
}

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_sidh_key_pair()
{
	//_info("SIDH generating...");
	PCurveIsogenyStaticData curveIsogenyData = &CurveIsogeny_SIDHp751;
	size_t obytes = (curveIsogenyData->owordbits + 7)/8; // Number of bytes in an element in [1, order]
	size_t pbytes = (curveIsogenyData->pwordbits + 7)/8; // Number of bytes in a field element
	const size_t private_key_len = obytes;
	const size_t public_key_len = 4*2*pbytes;
	locked_string private_key_a(private_key_len);
	locked_string private_key_b(private_key_len);
	std::string public_key_a(public_key_len, 0);
	std::string public_key_b(public_key_len, 0);
	CRYPTO_STATUS status = CRYPTO_SUCCESS;
	PCurveIsogenyStruct curveIsogeny = SIDH_curve_allocate(curveIsogenyData);
	try {
		if (curveIsogeny == nullptr) throw std::runtime_error("SIDH_curve_allocate error");
		status = SIDH_curve_initialize(curveIsogeny, &random_bytes_sidh, curveIsogenyData);
		// generate keys
		status = KeyGeneration_A(
			reinterpret_cast<unsigned char*>(&private_key_a[0]),
			reinterpret_cast<unsigned char *>(&public_key_a[0]),
			curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("private key generate error (A)");
		status = KeyGeneration_B(
			reinterpret_cast<unsigned char*>(&private_key_b[0]),
			reinterpret_cast<unsigned char *>(&public_key_b[0]),
			curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("private key generate error (B)");

		// check keys valid
		//_info("SIDH validate...");
		bool valid_pub_key = false;
		status = Validate_PKA(
		reinterpret_cast<unsigned char *>(&public_key_a[0]),
			&valid_pub_key,
			curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("validate public key error (A)");
		if (!valid_pub_key) throw std::runtime_error("public key (A) is not valid");
		status = Validate_PKB(
		reinterpret_cast<unsigned char *>(&public_key_b[0]),
			&valid_pub_key,
			curveIsogeny);
		if (status != CRYPTO_SUCCESS) throw std::runtime_error("validate public key error (B)");
		if (!valid_pub_key) throw std::runtime_error("public key (B) is not valid");
		assert(public_key_a != public_key_b);
		assert(private_key_a != private_key_b);
	}
	catch(const std::exception &e) {
		SIDH_curve_free(curveIsogeny);
		clear_words(static_cast<void*>(&private_key_a[0]), NBYTES_TO_NWORDS(private_key_len));
		clear_words(static_cast<void*>(&private_key_b[0]), NBYTES_TO_NWORDS(private_key_len));
		clear_words(static_cast<void*>(&public_key_a[0]), NBYTES_TO_NWORDS(public_key_len));
		clear_words(static_cast<void*>(&public_key_b[0]), NBYTES_TO_NWORDS(public_key_len));
		throw e;
	}
	SIDH_curve_free(curveIsogeny);
	locked_string private_key_main(2 * private_key_len);
	std::copy_n(private_key_a.begin(), private_key_len, private_key_main.begin());
	std::copy_n(private_key_b.begin(), private_key_len, private_key_main.begin() + private_key_len);
	std::string public_key_main = public_key_a + public_key_b;
	return std::make_pair(std::move(private_key_main), std::move(public_key_main));
}

c_multisign c_multikeys_PAIR::multi_sign(const string &msg) {
	c_multisign multi_signature = c_multisign();
	// all key crypto systems
	for (size_t sys=0; sys<m_PRV.get_count_of_systems(); ++sys) {

		auto crypto_type = int_to_enum<t_crypto_system_type>(sys); // enum of this crypto syste

		// crypto systems allowed for signing
		// or allowed crypto system is empty
		if (!c_multisign::cryptosystem_sign_allowed(crypto_type) ||
			m_PRV.get_count_keys_in_system(crypto_type) == 0) {
			continue;
		}

		std::vector<std::string> signatures = multi_sign(msg, crypto_type);
		multi_signature.add_signature_vec(signatures, crypto_type);
		//multi_signature.add_signature_vec(signatures, sys_enum);
	}
	return multi_signature;
}

std::vector<string> c_multikeys_PAIR::multi_sign(const string &msg,
												 t_crypto_system_type sign_type) {

	std::vector<std::string> signatures;

	size_t keys_count = m_PRV.get_count_keys_in_system(sign_type);
	assert(keys_count > 0 && "no keys of the required type");

	switch(sign_type) {
		case e_crypto_system_type_Ed25519: {
			for(size_t i = 0; i < keys_count; ++i) {
				std::string sign;
				sign = sodiumpp::crypto_sign_detached(msg,m_PRV.get_PRIVATE(sign_type,i).get_string());
				signatures.emplace_back(std::move(sign));
			}
			break;
		}
		case e_crypto_system_type_NTRU_sign: {
			for(size_t i = 0; i < keys_count; ++i) {
				std::string sign;
				auto PRV_key = m_PRV.get_PRIVATE(sign_type,i);
				sign = ntrupp::sign(msg, PRV_key);
				signatures.emplace_back(std::move(sign));
			}
			break;
		}
		default: throw std::runtime_error("sign type not supported");
	}

	return signatures;
}

void c_multikeys_PAIR::multi_sign_verify(const std::vector<string> &signs,
										 const string &msg,
										 const c_multikeys_pub &pubkeys,
										 t_crypto_system_type sign_type) {

	size_t amount_of_pubkeys = pubkeys.get_count_keys_in_system(sign_type);
	_dbg2("pubkeys: " << amount_of_pubkeys << " signatures: " << signs.size());
	assert(amount_of_pubkeys == signs.size() && "keys_size != signs_size");

	switch(sign_type) {
		case e_crypto_system_type_Ed25519: {
			for(size_t i = 0; i < amount_of_pubkeys; ++i) {
				std::string pubkey = pubkeys.get_public(sign_type,i);
				try {
					sodiumpp::crypto_sign_verify_detached(signs[i], msg, pubkey);
				} catch (sodiumpp::crypto_error &err) {
					throw std::invalid_argument(err.what());
				}
			}
			break;
		}
		case e_crypto_system_type_NTRU_sign: {
			for(size_t i = 0; i < amount_of_pubkeys; ++i) {
				std::string pubkey = pubkeys.get_public(sign_type,i);
				if (!ntrupp::verify(signs[i], msg, pubkey)) {
					throw std::invalid_argument("Ntru sign verify: fail");
				}
			}
			break;
		}
		default: throw std::invalid_argument("sign type not supported");
	}
}

void c_multikeys_PAIR::multi_sign_verify(const c_multisign &all_signatures,
										 const string &msg,
										 const c_multikeys_pub &pubkeys) {

	if (all_signatures.get_count_of_systems() != pubkeys.get_count_of_systems()) {
		throw std::invalid_argument("count of systems in c_multikeypub and c_multisign different!");
	}

	for (size_t sys=0; sys < all_signatures.get_count_of_systems(); ++sys) {

		auto crypto_type = int_to_enum<t_crypto_system_type>(sys); // enum of this crypto syste

		if (all_signatures.get_count_of_systems() != pubkeys.get_count_of_systems()) {
			std::string err_msg = "count of keys system [";
			err_msg += t_crypto_system_type_to_name(crypto_type);
			err_msg += "] in c_multikeypub and c_multisign different!";
			throw std::invalid_argument(err_msg);
		}

		// crypto systems allowed for signing
		if (!c_multisign::cryptosystem_sign_allowed(crypto_type)) {
			continue;
		}
		multi_sign_verify(all_signatures.get_signature_vec(crypto_type), msg, pubkeys, crypto_type);
	}
}

void c_multikeys_PAIR::generate(t_crypto_system_type crypto_system_type, int count) {
	if (!count) return;

	switch (crypto_system_type)
	{
		case e_crypto_system_type_X25519:
		{
			for (int i=0; i<count; ++i) {
				auto keypair = generate_x25519_key_pair();
				this->add_public_and_PRIVATE( crypto_system_type , keypair.second , keypair.first );
			}
			break;
		}

		case e_crypto_system_type_Ed25519:
		{
			for (int i=0; i<count; ++i) {
				auto keypair = generate_ed25519_key_pair();
				this->add_public_and_PRIVATE( crypto_system_type , keypair.second , keypair.first );
			}
			break;
		}

		case e_crypto_system_type_NTRU_EES439EP1:
		{
			for (int i=0; i<count; ++i) {
				auto keypair = generate_nrtu_encrypt_key_pair();
				this->add_public_and_PRIVATE( crypto_system_type , keypair.second , keypair.first );
			}
			break;
		}

		case e_crypto_system_type_NTRU_sign:
		{
			for (int i=0; i<count; ++i) {
				auto keypair = generate_nrtu_sign_key_pair();
				this->add_public_and_PRIVATE( crypto_system_type , keypair.second , keypair.first );
			}
			break;
		}

		case e_crypto_system_type_SIDH :
		{
			for (int i=0; i<count; ++i) {
				auto keypair = generate_sidh_key_pair();
				this->add_public_and_PRIVATE( crypto_system_type , keypair.second , keypair.first );
			}
			break;
		} // case

		default: throw runtime_error("Trying to generate unsupported key type:"
			+ t_crypto_system_type_to_name(crypto_system_type));
	} // switch

	string serialized = this->m_pub.serialize_bin();
	_info("Serialized pubkeys: [" << to_debug(serialized) << "]");
}

void c_multikeys_PAIR::add_public_and_PRIVATE(t_crypto_system_type crypto_type,
	 const c_crypto_system::t_pubkey & pubkey ,
	 const c_crypto_system::t_PRVkey & PRVkey)
{
	m_pub.add_public(crypto_type, pubkey);
	_info("Adding PRIVATE KEY: (" << t_crypto_system_type_to_name(crypto_type) << ") " << to_debug_locked(PRVkey));
	m_PRV.add_PRIVATE(crypto_type, PRVkey);
	//_note("ADD PRIVATE KEY: RESULT IS: " << to_debug(m_PRV.serialize_bin()));
}


const c_multikeys_pub & c_multikeys_PAIR::read_pub() const {
	return m_pub;
}



} // namespace




