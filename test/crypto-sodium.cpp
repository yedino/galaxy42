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