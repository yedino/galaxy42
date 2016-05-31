
#include "multikeys.hpp"

#include "crypto.hpp"

#include "../crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

#include "../build_extra/ntru/include/ntru_crypto.h"
#include <SIDH_internal.h>

#include "../trivialserialize.hpp"

#include "../filestorage.hpp"
#include "../text_ui.hpp"
#include "../ntrupp.hpp"

#include "../glue_lockedstring_trivialserialize.hpp"
#include "../glue_sodiumpp_crypto.hpp"



namespace antinet_crypto {

using sodiumpp::locked_string;


t_crypto_system_type c_multisign::get_system_type() const { return e_crypto_system_type_multisign; }

t_crypto_system_type c_multikeys_pub::get_system_type() const { return e_crypto_system_type_multikey_pub; }

t_crypto_system_type c_multikeys_PRV::get_system_type() const { return e_crypto_system_type_multikey_private; }

t_crypto_system_type c_multikeys_PAIR::get_system_type() const { return e_crypto_system_type_multikey_private; }


// ##################################################################
// ##################################################################
// all about the multikeys






// ==================================================================
// c_multicryptostrings<>

template <typename TKey>
c_multicryptostrings<TKey>::c_multicryptostrings(t_crypto_use crypto_use)
 : m_crypto_use( crypto_use )
{ }

template <typename TKey>
size_t c_multicryptostrings<TKey>::get_count_keys_in_system(t_crypto_system_type crypto_type) const {
	return m_cryptolists_general.at(crypto_type).size();
}

template <typename TKey>
size_t c_multicryptostrings<TKey>::get_count_of_systems() const {
	return m_cryptolists_general.size();
}

template <typename TKey>
bool c_multicryptostrings<TKey>::operator>(const t_self &rhs) const {
	return this->get_hash() > rhs.get_hash();
}

template <typename TKey>
std::string c_multicryptostrings<TKey>::get_hash() const {
	if (m_hash_cached=="") update_hash();
	assert(m_hash_cached != "");
	return m_hash_cached;
}

template <typename TKey>
void c_multicryptostrings<TKey>::update_hash() const {
	// TODO review this code, maybe pick nicer (easier to generate by others) format

	//m_hash_cached = Hash1( this->serialize_bin()  );
	t_crypto_use crypto_use_for_fingerprint = e_crypto_use_fingerprint;
	trivialserialize::generator gen(100);
	gen.push_bytes_n(3,"GMK"); // magic marker - GMK - "Galaxy MultiKey"
	gen.push_byte_u( (char) 'a' ); // version of this map
	gen.push_byte_u( crypto_use_for_fingerprint );
	gen.push_byte_u( 1 ); /// sub-version of fingerprinting format

	int used_types=0; // count how many key types are actually used - we will count below
	for (size_t ix=0; ix<m_cryptolists_general.size(); ++ix) if (m_cryptolists_general.at(ix).size()) ++used_types;
	gen.push_integer_uvarint(used_types); // save the size of crypto list (number of main elements)
	int used_types_check=0; // counter just to assert
	for (size_t ix=0; ix<m_cryptolists_general.size(); ++ix) { // for all key type (for each element)
		const vector<TKey> & pubkeys_of_this_system  = m_cryptolists_general.at(ix); // take vector of keys
		if (pubkeys_of_this_system.size()) { // save them this time
			++used_types_check;
			gen.push_byte_u(  t_crypto_system_type_to_ID(ix) ); // save key type

			// fill it with 0 bytes (octets):
			locked_string fpr_accum( Hash1_size() ); // accumulator to get XORed hashes of fprs of this type
			for (size_t p=0; p<fpr_accum.size(); ++p) fpr_accum[p] = static_cast<unsigned char>(0);

			// vector<string> fingerprints;
			for (const auto pk : pubkeys_of_this_system) {
				auto h = Hash1( pk );
				_dbg1("fingerprint: pk -> h: " << ::to_debug(pk) << " -> " << ::to_debug(h));
				using namespace string_binary_op;
				fpr_accum = fpr_accum ^ h;
				//fingerprints.push_back(h);
			}
			gen.push_varstring( fpr_accum.get_string() ); // save the "vector" of key FINGERPRINTS
		}
	}
	assert(used_types_check == used_types); // we written same amount of keys as we previously counted
	string fingerprint_serial = gen.str();

	string myhash = Hash1( fingerprint_serial );

	m_hash_cached =  myhash;
}

template <typename TKey>
void c_multicryptostrings<TKey>::add_key(t_crypto_system_type type, const t_key & pubkey) {
	auto & sys_vector = m_cryptolists_general.at( type );
	//_note("ADD KEY: size before: " << sys_vector.size());
	sys_vector.push_back( pubkey );
	//_note("ADD KEY: size after: " << sys_vector.size());
	//_note("ADD KEY: RESULT IS: " << to_debug(serialize_bin()));
	//_note("ADD KEY: THE newest element IS: " << to_debug_locked_maybe( sys_vector.at( sys_vector.size()-1 )  ));
}

template <typename TKey>
typename c_multicryptostrings<TKey>::t_key  c_multicryptostrings<TKey>::get_key(
t_crypto_system_type crypto_type, size_t number_of_key) const
{
	// TODO check range
	return m_cryptolists_general.at(crypto_type).at(number_of_key);
}



/**

Serialiation format

TODO(hb) this is comment that should be linked from the serialize_bin() function

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
, ,0x11(17),3,\,0xe0(224),0xdc(220),0xc7(199),H,0xed(237),0x81(129),X,0xd1(209),0xca(202),=,A,0x15(21),0xd3(211),V,0xc0(192),&,0x90(144),",X,0xe4(228),0xaa(170),0x92(146),0xa2(162),0x85(133),0xd6(214),2,0xb7(183),0xcd(205),0x04(04),

0x17(23) - now key type 23,
0x01(01) - 1 piece of it
, ,f,0x80(128),0x81(129),$,0x98(152),0x89(137),0x0f(15),0xde(222),0xbc(188),0xbc(188),0x1f(31),g,0xc9(201),R,6,0x94(148),j,0xd2(210),0x8f(143),0xd5(213),0x98(152),q,0xb7(183),3,[,0xb2(178),0xb4(180),0x8c(140),0xa5(165),0xdf(223),.,b]]
	*/

template <typename TKey>
std::string c_multicryptostrings<TKey>::serialize_bin() const { ///< returns a string with all our data serialized, to a binary format
	trivialserialize::generator gen(100);
	gen.push_bytes_n(3,"GMK"); // magic marker - GMK - "Galaxy MultiKey"
	gen.push_byte_u( (char) 'a' ); // version of this map. '$' will be development, and then use 'a','b',... for stable formats
	gen.push_byte_u( m_crypto_use ); // marker is it open or secret
	int used_types=0; // count how many key types are actually used - we will count below
	for (size_t ix=0; ix<m_cryptolists_general.size(); ++ix) if (m_cryptolists_general.at(ix).size()) ++used_types;
	gen.push_integer_uvarint(used_types); // save the size of crypto list (number of main elements)
	int used_types_check=0; // counter just to assert
	for (size_t ix=0; ix<m_cryptolists_general.size(); ++ix) { // for all key type (for each element)
		const vector<TKey> & pubkeys_of_this_system  = m_cryptolists_general.at(ix); // take vector of keys
		if (pubkeys_of_this_system.size()) { // save them this time
			++used_types_check;
			gen.push_integer_uvarint(  t_crypto_system_type_to_ID(ix) ); // save key type
			gen.push_vector_object( pubkeys_of_this_system ); // save the vector of keys
		}
	}
	assert(used_types_check == used_types); // we written same amount of keys as we previously counted
	return gen.str();
}

template <typename TKey>
void c_multicryptostrings<TKey>::load_from_bin(const std::string & data) {
	// clear(); // remove all keys TODO

	trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_string_valid() , data );
	auto magic_marker = parser.pop_bytes_n(3);
	if (magic_marker !=  "GMK") throw

		std::runtime_error( string("Format incorrect: bad magic marker for GMK (was: ")
			+ ::to_debug(magic_marker) + string(")"));

	auto magic_version = parser.pop_byte_u();

	auto magic_crypto_use = parser.pop_byte_u();
	if (magic_crypto_use != m_crypto_use) {
		std::ostringstream oss;
		oss<<"Format error: crypto_use was=" << ::to_debug(magic_crypto_use) << " but we expected=" << m_crypto_use;
		throw std::runtime_error(oss.str());
	}

	if (magic_version == 'a') {
		size_t map_size = parser.pop_integer_uvarint();
		assert( map_size <= 100 ); // TODO(serialize_parser_assert)
		for (size_t map_i=0; map_i<map_size; ++map_i) {
			auto sys_id = t_crypto_system_type_from_ID(  parser.pop_integer_uvarint() );
			_info("sys_id=" << sys_id);
			auto sys_keys = parser.pop_vector_object<TKey>(); // <--- load vector of all keys of this system
			// TODO(r) asert sys_id is a normal expected crypto key type
			this->m_cryptolists_general.at( sys_id ) = sys_keys;
		}
		if (!parser.is_end()) throw std::runtime_error("Format incorrect: extra elements at end");
	}	else throw trivialserialize::format_error_read_invalid_version();
	// TODO(r) check that numbers are sorted and not-repeating; extent exceptions type to report details of problem
	this->m_hash_cached=""; // mark it as dirty
}

template <typename TKey>
void c_multicryptostrings<TKey>::datastore_save(const string  & fname, bool overwrite) const {
try {
	// TODO need a serialize_bin() that works on, and returns, a locked_string
	_note("Savin key to fname="<<fname);

	std::string serialized_data = serialize_bin();
	_info("Serialized to: " << to_debug_locked_maybe(serialized_data));

	switch(m_crypto_use) {
		case e_crypto_use_open: {
			_note("Save this as public key");
			filestorage::save_string(e_filestore_galaxy_ipkeys_pub, fname, serialized_data, overwrite);

			break;
		}
		case e_crypto_use_secret: {
			_note("Save this as PRIVATE key!!!");
			locked_string data = locked_string::unsafe_create(serialized_data);
			filestorage::save_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, fname, data, overwrite);
			break;
		}
		default:
			throw std::runtime_error("Can not handle this crypto_use");
	}

} catch(overwrite_error &err) {
	std::cout << err.what() << std::endl;
	if(text_ui::ask_user_forpermission("overwrite file?")){
		this->datastore_save(fname, true);
	}
}

}

template <typename TKey>
void c_multicryptostrings<TKey>::datastore_load(const string  & fname) {
	std::string data;
	locked_string buff_safe;

	switch(m_crypto_use) {
		case e_crypto_use_open: {
			_note("Load this as public key");
			data = filestorage::load_string(e_filestore_galaxy_ipkeys_pub, fname);
			clear();
			_info("Loading: reading now");
			load_from_bin(data);
			break;
		}
		case e_crypto_use_secret: {
			_note("Load this as PRIVATE key!!!");
			buff_safe = filestorage::load_string_mlocked(e_filestore_wallet_galaxy_ipkeys_PRV, fname);
			clear();
			_info("Loading: reading now");
			load_from_bin(buff_safe.get_string());
			break;
		}
		default:
			throw std::runtime_error("Can not handle this crypto_use");
	}
	_info("Loading: done, debug: " << to_debug_locked_maybe(serialize_bin()));
}

template <typename TKey>
void c_multicryptostrings<TKey>::clear() {
	for (auto & sys : m_cryptolists_general) sys.clear();
}

template <typename TKey>
std::string c_multicryptostrings<TKey>::to_debug() const {
	ostringstream oss;
	oss << "MK(";
	for (size_t ix=0; ix<m_cryptolists_general.size(); ++ix) { // for all key type (for each element)
		const vector<TKey> & pubkeys_of_this_system  = m_cryptolists_general.at(ix); // take vector of keys
		if (pubkeys_of_this_system.size()) {
			oss << t_crypto_system_type_to_name(ix);
			oss <<  " *" << pubkeys_of_this_system.size();
		}
	}
	oss << ")";
	return oss.str();
}

// ==================================================================
// c_multikeys_general

template <typename TKey>
c_multikeys_general<TKey>::c_multikeys_general(t_crypto_use crypto_use)
	: c_multicryptostrings<TKey>(crypto_use)
{ }









// ##################################################################
// ##################################################################





// TODONOW remove?
void instantiate() {
	{ c_multicryptostrings<std::string> someobject(e_crypto_use_open); }
	{ c_multicryptostrings<sodiumpp::locked_string> someobject(e_crypto_use_open); }
	{ c_multikeys_general<std::string> someobject(e_crypto_use_open); }
	{ c_multikeys_general<sodiumpp::locked_string> someobject(e_crypto_use_open); }
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

std::pair<sodiumpp::locked_string, string> c_multikeys_PAIR::generate_nrtu_key_pair() {
	// generate key pair
	uint16_t public_key_len = 0, private_key_len = 0;
	// get size of keys:
// TODO use ntrupp
//	NTRU_exec_or_throw(
//		ntru_crypto_ntru_encrypt_keygen(
//			get_DRBG(128),
//			NTRU_EES439EP1,
//			&public_key_len, nullptr, &private_key_len, nullptr
//			)
//		,"generate keypair - get key length"
//	);
	// values for NTRU_EES439EP1
	assert(public_key_len == 609);
	assert(private_key_len == 659);

	std::string public_key(public_key_len, 0);
	locked_string private_key(private_key_len);

// TODO use ntrupp
//	NTRU_exec_or_throw(
//		ntru_crypto_ntru_encrypt_keygen(get_DRBG(128), NTRU_EES439EP1,
//			&public_key_len, reinterpret_cast<uint8_t*>(&public_key[0]),
//			&private_key_len, reinterpret_cast<uint8_t*>(private_key.buffer_writable())
//		)
//		,"generate keypair"
//	);

	return std::make_pair(std::move(private_key), std::move(public_key));
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

std::vector<string> c_multikeys_PAIR::multi_sign(const string &msg,
												 t_crypto_system_type sign_type) {

	std::vector<std::string> signs;

	size_t keys_count = m_PRV.get_count_keys_in_system(sign_type);
	assert(keys_count > 0 && "no keys of the required type");

	switch(sign_type) {
		case e_crypto_system_type_Ed25519: {
			for(size_t i = 0; i < keys_count; ++i) {
				std::string sign;
				sign = sodiumpp::crypto_sign_detached(msg,m_PRV.get_PRIVATE(sign_type,i).get_string());
				signs.emplace_back(std::move(sign));
			}
			break;
		}
		case e_crypto_system_type_NTRU_EES439EP1: {
			for(size_t i = 0; i < keys_count; ++i) {
				std::string sign;
				auto pubkey = m_PRV.get_PRIVATE(sign_type,i).get_string();
			}
			break;
		}
		default: throw std::runtime_error("sign type not supported");
	}

	return signs;
}

void c_multikeys_PAIR::multi_sign_verify(const std::vector<string> &signs,
										 const string &msg,
										 const c_multikeys_pub &pubkeys,
										 t_crypto_system_type sign_type) {

	size_t keys_size = pubkeys.get_count_keys_in_system(sign_type);
	assert(keys_size == signs.size() && "keys_size != signs_size");

	switch(sign_type) {
		case e_crypto_system_type_Ed25519: {
			for(size_t i = 0; i < keys_size; ++i) {
				std::string pubkey = pubkeys.get_public(sign_type,i);
				sodiumpp::crypto_sign_verify_detached(signs.at(i), msg, pubkey);
			}
			break;
		}
		default: throw std::runtime_error("sign type not supported");
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
				auto keypair = generate_nrtu_key_pair();
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




