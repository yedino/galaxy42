
#pragma once
#ifndef include_crypto_multikeys_tpl_hpp
#define include_crypto_multikeys_tpl_hpp 1

#include "multikeys.hpp"


namespace antinet_crypto {

	using sodiumpp::locked_string;

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
void c_multicryptostrings<TKey>::set_hash_dirty() {
	m_hash_cached=""; // mark it as dirty
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
	set_hash_dirty();
}

template <typename TKey>
void c_multicryptostrings<TKey>::datastore_save(const string  & fname, bool overwrite) const {
	// TODO need a serialize_bin() that works on, and returns, a locked_string
	_note("Saving key to fname="<<fname);

	std::string serialized_data = serialize_bin();
	_info("Serialized to: " << to_debug_locked_maybe(serialized_data));

	switch(m_crypto_use) {
		case e_crypto_use_open: {
			_note("Save this as public key");
			filestorage::save_string(e_filestore_galaxy_pub, fname, serialized_data, overwrite);

			break;
		}
		case e_crypto_use_signature: {
			_note("Save this as signature");
			filestorage::save_string(e_filestore_galaxy_sig, fname, serialized_data, overwrite);
		  }
		case e_crypto_use_secret: {
			_note("Save this as PRIVATE key!!!");
			locked_string data = locked_string::unsafe_create(serialized_data);
			filestorage::save_string_mlocked(e_filestore_galaxy_wallet_PRV, fname, data, overwrite);

			break;
		}
		default:
			throw std::runtime_error("Can not handle this crypto_use");
	}
}

template <typename TKey>
void c_multicryptostrings<TKey>::datastore_load(const string  & fname) {
	std::string data;
	locked_string buff_safe;

	switch(m_crypto_use) {
		case e_crypto_use_open: {
			_note("Load this as public key");
			data = filestorage::load_string(e_filestore_galaxy_pub, fname);
			clear();
			_info("Loading: reading now");
			load_from_bin(data);
			break;
		}
		case e_crypto_use_secret: {
			_note("Load this as PRIVATE key!!!");
			buff_safe = filestorage::load_string_mlocked(e_filestore_galaxy_wallet_PRV, fname);
			clear();
			_info("Loading: reading now");
			load_from_bin(buff_safe.get_string());
			break;
		}
		case e_crypto_use_signature: {
			_note("Load this as signature");
			data = filestorage::load_string(e_filestore_galaxy_sig, fname);
			clear();
			_info("Loading: reading now");
			load_from_bin(data);
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

} // namespace antinet_crypto


#endif


