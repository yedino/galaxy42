
#include "galaxy_debug.hpp"

#include "libs0.hpp"
#include "strings_utils.hpp"

// #include "glue_lockedstring_trivialserialize.hpp
// #include "glue_sodiumpp_crypto.hpp"

#include "crypto/crypto_basic.hpp" // for the to_debug_locked_maybe


namespace unittest {


#include "strings_utils.hpp"


void test_debug1_a_string(const string & input) {
	std::string s( input );
	sodiumpp::locked_string sl(s);


	_note("string: " << ::to_debug(s));
	_info("string: " << antinet_crypto::to_debug_locked_maybe(s));
	_info("string locked: " << antinet_crypto::to_debug_locked(sl));
	_info("string locked: " << antinet_crypto::to_debug_locked_maybe(sl));


	_info("string: " << ::to_debug(s,e_debug_style_crypto_devel));
	_info("string: " << antinet_crypto::to_debug_locked_maybe(s));
	_info("string locked: " << antinet_crypto::to_debug_locked(sl));
	_info("string locked: " << antinet_crypto::to_debug_locked_maybe(sl));

}


void test_debug1() {
	_dbg3("dbg3");
	_dbg2("dbg2");
	_dbg1("dbg1");
	_info("info");
	test_debug1_a_string("foo\001\002\003bar");
	test_debug1_a_string("foo\001\002\003baz");
	test_debug1_a_string("foo\001\002\003baraaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	test_debug1_a_string("foo\001\002\003bazaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
}


} // namespace

