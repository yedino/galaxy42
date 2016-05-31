#include "gtest/gtest.h"
#include "../crypto.hpp"
#include "../ntrupp.hpp"

// ntru sign
extern "C" {
#include <constants.h>
#include <pass_types.h>
#include <hash.h>
#include <ntt.h>
#include <pass.h>
}

#include "../ntrupp.hpp"

namespace antinet_crypto {

/* broke after the memlock and _PRV hash functions...
TEST(crypto, aeshash_not_repeating_state_nor_password) {
	std::set< t_hash > used_hash;
	const int amount_iterations = 10000;

    enum class type_RX : int { RX_none=0, RX_constant, RX_same, RX_random , RX_END };


    for (auto rx_type = type_RX::RX_none ; rx_type < type_RX::RX_END ; rx_type = static_cast<type_RX>(static_cast<int>(rx_type) + 1)  ) {
		c_symhash_state symhash( "" );

		auto rx_same = symhash.secure_random( 10 );

		switch (rx_type) { // first nextstate after creation of symhash
            case type_RX::RX_none:			break; // here we do not do it, but must be then done in others to avoid collision
            case type_RX::RX_constant:		symhash.next_state( "foo" );
                                            break;
            case type_RX::RX_same:			symhash.next_state( rx_same );
                                            break;
            case type_RX::RX_random:		symhash.next_state( symhash.secure_random( 2 ) );
                                            break;
            default: assert(false);
		}


		for (int i=0; i<amount_iterations; ++i) {
			{
				auto result = used_hash.insert( symhash.get_the_SECRET_PRIVATE_state() );
				ASSERT_TRUE( result.second == true ); // inserted new one
			}
			{
				auto result = used_hash.insert( symhash.get_password() );
				ASSERT_TRUE( result.second == true ); // inserted new one
			}
			switch (rx_type) {
                case type_RX::RX_none:		symhash.next_state();
                                            break;
                case type_RX::RX_constant:	symhash.next_state( "foo" );
                                            break;
                case type_RX::RX_same:		symhash.next_state( rx_same );
                                            break;
                case type_RX::RX_random:	symhash.next_state( symhash.secure_random( 2 ) );
                                            break;
				default: assert(false);
			}
		} // all iterations of using it


		switch (rx_type) {
            case type_RX::RX_none:
				EXPECT_EQ( string_as_hex(symhash.get_password()) , string_as_hex("084b0ff5a81c8f3c1001b2d596cc02db629ea047716eba8440bb823223f18bddaa3631a02e43bbf886584cc636eb0a56a5813f15c9c0aeb3b5b4b4877221da8e") );
				EXPECT_EQ( string_as_hex(symhash.get_the_SECRET_PRIVATE_state()) , string_as_hex("b64bde9d13b26847df387b6aa2f475a1309b64d14aaa07877df1b43cd1c79364ff7d7fbbef222ec41d55bb21f4144124c91d69a7411d3a4e29178a7e6748e097") );
			break;
			default: break;
		}
	}
}
*/

TEST(crypto, aeshash_start_and_get_same_passwords) {
	c_symhash_state symhash( string_as_bin(string_as_hex("6a6b")).bytes ); // "jk"
	string_as_hex p(( string_as_bin( symhash.get_password() ) ));
	//	cout << "\"" << string_as_hex( symhash.get_password() ) << "\"" << endl;
	EXPECT_EQ(p.get(), "1ddb0a828c4d3776bf12abbe17fb4d82bcaf202a1b00b5b54e90db701303d69ce235f36d25c9fd1343225888e00abdc0e18c2036e86af9f3a90faf1abfefedf7");
	symhash.next_state();

	p = string_as_hex( symhash.get_password() );
	EXPECT_EQ(p.get() ,"72e4af0f04e2113852fd0d5320a14aeb2219d93ed710bc9bd72173b4ca657f37e4270c8480beb8fded05b6161d32a6450d4c3abb86023984f4f9017c309b5330");

	symhash.next_state();
	p = string_as_hex( symhash.get_password() );
	EXPECT_EQ(p.get(), "8a986c419f1347d8ea94b3ad4b9614d840bb2dad2e13287a7a6cb5cf72232c3211997b6435f44256a010654d6f49e71517e46ce420a77f09f3a425eabaa99d8a");
}

TEST(crypto, dh_exchange) {
	/* TODO c_dhdh_state will be removed probably...
	auto alice_keys = c_dhdh_state::generate_key_pair();
	auto alice_pub = alice_keys.first;
	auto alice_priv = alice_keys.second;
	EXPECT_EQ(alice_pub, alice_keys.first);
	EXPECT_EQ(alice_priv, alice_keys.second);

	auto bob_keys = c_dhdh_state::generate_key_pair();
	auto bob_pub = bob_keys.first;
	auto bob_priv = bob_keys.second;
	EXPECT_EQ(bob_pub, bob_keys.first);
	EXPECT_EQ(bob_priv, bob_keys.second);

	EXPECT_NE(alice_pub, bob_pub);
	EXPECT_NE(alice_priv, bob_priv);

	EXPECT_NE(alice_pub, alice_priv);
	EXPECT_NE(bob_pub, bob_priv);

	c_dhdh_state alice_state(alice_priv, alice_pub, bob_pub);
	c_dhdh_state bob_state(bob_priv, bob_pub, alice_pub);

	auto alice_sym_key = alice_state.execute_DH_exchange();
	auto bob_sym_key = bob_state.execute_DH_exchange();
	EXPECT_EQ(alice_sym_key, bob_sym_key);
	*/
}

} // namespace

TEST(crypto, bin_string_xor) {

	std::string test_str("longrandomstring123123345345!@#$%^&*())))(*&^%$#@");
	std::string result;

	// xor for the same string should be tab of zeros
	result = antinet_crypto::string_binary_op::binary_string_xor(test_str, test_str);
	for(auto ch : result) {
		EXPECT_EQ(static_cast<int>(ch), 0);
	}

	// two empty strings
	result = antinet_crypto::string_binary_op::binary_string_xor(std::string(""),std::string(""));
	EXPECT_EQ(result.length(),0u);
	EXPECT_EQ(result,"");

	bool size_diff_err = false;
  try {
	// binary string xor of two string with different size should throw runtime error
	antinet_crypto::string_binary_op::binary_string_xor(std::string("xx"), std::string("xxxx"));
  } catch(std::runtime_error) {
		size_diff_err = true;
  }
	EXPECT_EQ(true, size_diff_err);

	size_diff_err = false;
  try {
	// binary string xor of two string with different size should throw runtime error
	// try with empty string
	antinet_crypto::string_binary_op::binary_string_xor(std::string(""), std::string("xxxx"));
  } catch(std::runtime_error) {
		size_diff_err = true;
  }
	EXPECT_EQ(true, size_diff_err);
}

TEST(crypto, ntru_sign) {
	const std::string msg("message to sign");
	int64 secretkey[PASS_N];
	int64 pubkey[PASS_N] = {0};
	int64 z[PASS_N];
	if(ntt_setup() == -1) {
		fprintf(stderr,
		"ERROR: Could not initialize FFTW. Bad wisdom?\n");
		exit(EXIT_FAILURE);
	}
	gen_key(secretkey);
	unsigned char hash[HASH_BYTES];
	crypto_hash_sha512(hash, reinterpret_cast<unsigned char*>(secretkey), sizeof(int64)*PASS_N); // necessary?
	gen_pubkey(pubkey, secretkey);
	sign(hash, z, secretkey, reinterpret_cast<const unsigned char *>(msg.data()), msg.size());
	ASSERT_EQ(verify(hash, z, pubkey, reinterpret_cast<const unsigned char *>(msg.data()), msg.size()), VALID);
	z[0] = ~ z[0];
	ASSERT_NE(verify(hash, z, pubkey, reinterpret_cast<const unsigned char *>(msg.data()), msg.size()), VALID);
	ntt_cleanup();

}

TEST(crypto, ntrupp_generate_keypair) {
	const size_t test_number = 5;
	{
		std::vector<decltype (ntrupp::generate_encrypt_keypair())> keys;
		for (size_t i = 0; i < test_number; ++i) {
			auto key_pair = ntrupp::generate_encrypt_keypair();
			keys.emplace_back(std::move(key_pair));
		}
		std::unique(keys.begin(), keys.end());
		ASSERT_EQ(keys.size(), test_number);
		keys.clear();
	}
	{
		std::vector<decltype (ntrupp::generate_sign_keypair())> keys;
		for (size_t i = 0; i < test_number; ++i) {
			auto key_pair = ntrupp::generate_sign_keypair();
			keys.emplace_back(std::move(key_pair));
		}
		std::unique(keys.begin(), keys.end());
		ASSERT_EQ(keys.size(), test_number);
	}
}

TEST(crypto, ntrupp_encrypt) {
	const size_t test_number = 100;
	std::string msg = "msg_to_encrypt/decrypt /.,l;']/n/t!@#$%^&*()_NULL_+0'0\0";

	// helping function that adding int to string
	auto iterate_string = [] (std::string &str, int add) -> std::string {
		std::string out = "";
		for(size_t i = 0; i < str.size(); ++i) {
			out += str.at(i) + add;
		}
		return out;
	};

	for (size_t i = 0; i < test_number; ++i) {
		std::string i_msg = iterate_string(msg, i);

		std::pair<sodiumpp::locked_string, std::string> l_keypair;
		l_keypair = ntrupp::generate_encrypt_keypair();

		std::string cyphertext = ntrupp::encrypt(i_msg, l_keypair.second);
		std::string decrypted(ntrupp::decrypt(cyphertext, l_keypair.first));

		ASSERT_EQ(i_msg, decrypted);
	}
}

TEST(crypto, ntrupp_sign) {

	FILE * f_ptr;
	f_ptr = std::fopen("data/769_wisdom.dat", "r");

	assert(f_ptr != NULL);

	std::pair<sodiumpp::locked_string, std::string> keypair = ntrupp::generate_sign_keypair();

	std::string msg_to_sign = "Message to sign";
	std::string signature;

	signature = ntrupp::sign(msg_to_sign, keypair.first);	// keys for encrypt ...
	std::cout << "Signature: " << signature << std::endl;
}

TEST(crypto, multi_sign_ed25519) {

	antinet_crypto::c_multikeys_PAIR Alice;
	Alice.generate(antinet_crypto::e_crypto_system_type_Ed25519,5);

	std::string msg_to_sign = "message";
	std::vector<std::string> signs;

	// ed25519 is default sign system
	signs = Alice.multi_sign(msg_to_sign);

	EXPECT_THROW({
		antinet_crypto::c_multikeys_PAIR::multi_sign_verify(signs,
															"bad_msg",
															Alice.read_pub());

	}, sodiumpp::crypto_error);

	EXPECT_NO_THROW( {
		antinet_crypto::c_multikeys_PAIR::multi_sign_verify(signs,
															msg_to_sign,
															Alice.read_pub());
	});
}
