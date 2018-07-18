// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "galaxy_debug.hpp"

#include "libs0.hpp"
#include "strings_utils.hpp"

// #include "glue_lockedstring_trivialserialize.hpp
// #include "glue_sodiumpp_crypto.hpp"

#include "crypto/crypto_basic.hpp" // for the to_debug_locked_maybe


namespace unittest {


#include "strings_utils.hpp"


void test_debug1_a_string(const std::string & input) {
	std::string s( input );
	sodiumpp::locked_string sl(s);


	pfp_note("string: " << ::to_debug(s));
	pfp_info("string: " << antinet_crypto::to_debug_locked_maybe(s));
	pfp_info("string locked: " << antinet_crypto::to_debug_locked(sl));
	pfp_info("string locked: " << antinet_crypto::to_debug_locked_maybe(sl));


	pfp_info("string: " << ::to_debug(s,e_debug_style_crypto_devel));
	pfp_info("string: " << antinet_crypto::to_debug_locked_maybe(s));
	pfp_info("string locked: " << antinet_crypto::to_debug_locked(sl));
	pfp_info("string locked: " << antinet_crypto::to_debug_locked_maybe(sl));

}


void test_debug1() {
	pfp_dbg3("dbg3");
	pfp_dbg2("dbg2");
	pfp_dbg1("dbg1");
	pfp_info("info");
	test_debug1_a_string("foo\001\002\003bar");
	test_debug1_a_string("foo\001\002\003baz");
	test_debug1_a_string("foo\001\002\003baraaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
	test_debug1_a_string("foo\001\002\003bazaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
}


} // namespace

