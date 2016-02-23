#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <exception>

#include "coinsign_error.hpp"

#include "c_netuser.hpp"
#include "../../crypto_ops/crypto/crypto_ed25519.hpp"

#include "ptest/ptest.hpp"

bool chrono_time();

bool test_manyEdSigning(int number_of_threads, size_t signs_num, size_t message_len);
bool test_user_sending ();
bool test_many_users ();
bool test_cheater();
bool test_fast_cheater();
bool test_malignant_cheater();

bool test_bad_chainsign();
bool test_convrt_tokenpacket();
bool test_netuser();

bool test_wallet_expected_sender();
bool test_wallet_mint_check();
int test_mint_token_expiration();
bool test_recieve_deprecated_token();

bool test_coinsign_error();

bool test_rpcwallet();

bool test_all(int number_of_threads);


#endif // TESTS_HPP
