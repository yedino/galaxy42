
#include "crypto_basic.hpp"


namespace antinet_crypto {

// ==================================================================
// extra implementation tools

bool safe_string_cmp(const std::string & a, const std::string & b) {
	if (a.size() != b.size()) return false;
	return 0 == sodium_memcmp( a.c_str() , b.c_str() , a.size() );
}

sodiumpp::locked_string substr(const sodiumpp::locked_string & str , size_t len) {
	if (len<1) throw std::runtime_error( string("Invalid substring of len=") + std::to_string(len) );
	sodiumpp::locked_string ret( len );
	assert(ret.size() == len);
	assert(len < str.size());
	for (size_t p=0; p<str.size() && p < ret.size(); ++p) ret.at(p) = str.at(p);
	return ret;
}



// ==================================================================
// debug

std::string to_debug_locked(const sodiumpp::locked_string & data) {
	#if OPTION_DEBUG_SHOW_SECRET_STRINGS
		return to_debug(data.get_string() , e_debug_style_crypto_devel);
	#else
		UNUSED(data);
	#endif
	return "[hidden-secret](locked_string)";
}


std::string to_debug_locked_maybe(const sodiumpp::locked_string & data) {
	return to_debug_locked(data);
}
std::string to_debug_locked_maybe(const std::string & data) {
	return to_debug(data);
}

// ==================================================================


// ==================================================================
namespace string_binary_op {


std::string operator^(const std::string & str1, const std::string & str2) {
	return binary_string_xor(str1,str2);
}

sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const sodiumpp::locked_string & str2) {
	return binary_string_xor(str1,str2);
}

sodiumpp::locked_string operator^(const sodiumpp::locked_string & str1, const std::string & str2_un) {
	sodiumpp::locked_string str2(str2_un);
	return binary_string_xor(str1,str2);
}


} // namespace
// ==================================================================



// ==================================================================

// === The crypto types ===

t_crypto_system_type c_crypto_system::get_system_type() const { return e_crypto_system_type_invalid; }


// ==================================================================
// Random

CRYPTO_STATUS random_bytes_sidh(unsigned int nbytes, unsigned char *random_array) {
	// DELETE ME
	return CRYPTO_SUCCESS;
}

// ==================================================================

t_hash Hash1( const t_hash & hash ) {
    // TODO I know this look horrible, we should implement some (unsigned char <-> char) wrapper
    size_t u_hashmsg_len = hash.length();
    const unsigned char* u_hashmsg;
    u_hashmsg = reinterpret_cast<const unsigned char *>(&hash[0]);

   	const size_t out_u_hash_len = 64; // Hash1_size();
   	assert( out_u_hash_len == Hash1_size() );  // <-- ^-- TODO(rob) constexpr instead?

    assert( out_u_hash_len <=  crypto_generichash_BYTES_MAX );
    unsigned char out_u_hash[out_u_hash_len];

    crypto_generichash(out_u_hash, out_u_hash_len,
                       u_hashmsg, u_hashmsg_len,
                       nullptr, 0);

    return string(reinterpret_cast<char *>(out_u_hash),  out_u_hash_len);
}

t_hash_PRV Hash1_PRV( const t_hash_PRV & hash ) {
    size_t u_hashmsg_len = hash.size();
    const unsigned char* u_hashmsg;
    u_hashmsg = reinterpret_cast<const unsigned char *>( hash.c_str() ); // read locked

   	const size_t out_u_hash_len = 64; // Hash1_size();
   	assert( out_u_hash_len == Hash1_size() );  // <-- ^-- TODO(rob) constexpr instead?

    assert( out_u_hash_len <=  crypto_generichash_BYTES_MAX );
		t_hash_PRV out_u_hash(out_u_hash_len);

    crypto_generichash(
			reinterpret_cast<unsigned char*>( out_u_hash.buffer_writable() ), // write locked
			out_u_hash_len,
      u_hashmsg, u_hashmsg_len, nullptr, 0);

    return out_u_hash;
}


size_t Hash1_size() {
	return 64;
}

t_hash Hash2( const t_hash & hash ) {
    t_hash hash_from_hash = Hash1(hash);
    for(auto &ch : hash_from_hash) { // negate all octets in it
        ch = ~ch;
    }
    const auto ret = Hash1(hash_from_hash);
    assert( ret.size() == Hash2_size() );
    return ret;
}

t_hash_PRV Hash2_PRV( const t_hash_PRV & hash ) {
    t_hash_PRV hash_from_hash = Hash1_PRV(hash);
    //for(auto &ch : hash_from_hash) { // negate all octets in it
		for (size_t p=0; p<hash_from_hash.size(); ++p) { // TODO use foreach when locked_string supports it
			char & ch = hash_from_hash.at(0); // TODO performance, change to [] after review
    	ch = ~ch;
    }
    const auto ret = Hash1_PRV(hash_from_hash);
    assert( ret.size() == Hash2_size() );
    return ret;
}

size_t Hash2_size() {
	return 64;
}

// ==================================================================

std::string t_crypto_system_type_to_name(int val) {
	switch(val) {
		case e_crypto_system_type_X25519:			return "X25519";
		case e_crypto_system_type_Ed25519:			return "Ed25519";
		case e_crypto_system_type_NTRU_EES439EP1:     return "NTRU-EES439EP1";
		case e_crypto_system_type_NTRU_sign:     return "NTRU-sign";
		case e_crypto_system_type_SIDH:			return "SIDH-p751";
		case e_crypto_system_type_geport_todo:			return "geport_todo";
		case e_crypto_system_type_symhash_todo:			return "symhash_todo";
		case e_crypto_system_type_multikey_pub:			return "multikey-pub";
		case e_crypto_system_type_multikey_private:			return "multikey-PRIVATE";
		// TODONOW(hb)
			//default:		return "Wrong type";
	}
	return string("(Invalid enum type=") + std::to_string(val) + string(")");
}

std::string enum_name(t_crypto_system_type e) {
	return t_crypto_system_type_to_name( e );
}

char t_crypto_system_type_to_ID(int val) {
	switch(val) {
		case e_crypto_system_type_X25519: return 'x';
		case e_crypto_system_type_Ed25519: return 'e';
		case e_crypto_system_type_NTRU_EES439EP1: return 't';
		case e_crypto_system_type_NTRU_sign: return 'r';
		case e_crypto_system_type_SIDH: return 's';
		case e_crypto_system_type_geport_todo: return 'g';
	}
	throw std::invalid_argument("[" + std::string(__func__) + "] Unknown crypto type (val == " + std::to_string(val) + ")");
}

t_crypto_system_type t_crypto_system_type_from_ID(char name) {
	switch(name) {
		case 'x': return e_crypto_system_type_X25519;
		case 'e': return e_crypto_system_type_Ed25519;
		case 't': return e_crypto_system_type_NTRU_EES439EP1;
		case 'r': return e_crypto_system_type_NTRU_sign;
		case 's': return e_crypto_system_type_SIDH;
		case 'g': return e_crypto_system_type_geport_todo;
	}
	throw std::invalid_argument("[" + std::string(__func__) + "] Unknown crypto type (name == " + name + ")");
}

bool t_crypto_system_type_is_asymkex(t_crypto_system_type sys) {
	switch (sys) {
		case e_crypto_system_type_X25519: return false;
		case e_crypto_system_type_SIDH: return false;
		case e_crypto_system_type_NTRU_EES439EP1: return true;
		default: break;	// other things are not a KEX at all:
	}
	throw std::invalid_argument("[" + std::string(__func__) + "] Unknown crypto type (name == " 
		+ t_crypto_system_type_to_name(sys) + ")");
}






} // namespace antinet_crypto

