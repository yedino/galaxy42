#include "gtest/gtest.h"
#include "../crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"

using namespace ecdh_ChaCha20_Poly1305;

TEST(crypto_sodium, generate_unique_key_pair) {
	std::set<privkey_t> priv_keys;
	std::set<pubkey_t> pub_keys;
	const size_t amount_iterations = 10000;
	for (size_t i = 0; i < amount_iterations; ++i) {
		keypair_t key_pair = generate_keypair();
		ASSERT_NE(key_pair.privkey, key_pair.pubkey);
		{
			auto result = priv_keys.insert(key_pair.privkey);
			ASSERT_TRUE(result.second);
		}
		{
			auto result = pub_keys.insert(key_pair.pubkey);
			ASSERT_TRUE(result.second);
		}
	}
	ASSERT_EQ(priv_keys.size(), amount_iterations);
	ASSERT_EQ(pub_keys.size(), amount_iterations);
}

TEST(crypto_codium, generate_sharedkey) {
	const size_t amount_iterations = 10000;
	std::set<sharedkey_t> shared_keys;
	for (size_t i = 0; i < amount_iterations; ++i) {
		keypair_t alice_keys = generate_keypair();
		keypair_t bob_keys = generate_keypair();
		EXPECT_NE(alice_keys.privkey, bob_keys.privkey);
		EXPECT_NE(alice_keys.pubkey, bob_keys.pubkey);
		sharedkey_t alice_shared_key = generate_sharedkey_with(alice_keys, bob_keys.pubkey);
		sharedkey_t bob_shared_key = generate_sharedkey_with(bob_keys, alice_keys.pubkey);
		ASSERT_EQ(alice_shared_key, bob_shared_key);
		// check unique shared_keys
		auto result = shared_keys.insert(alice_shared_key);
		ASSERT_TRUE(result.second);
	}
	EXPECT_EQ(shared_keys.size(), amount_iterations);
}