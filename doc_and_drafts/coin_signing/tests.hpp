#ifndef TESTS_HPP
#define TESTS_HPP

#include "libs01.hpp"
#include "coinsign_error.hpp"
#include "c_netuser.hpp"
#include "c_json_serializer.hpp"

#include "ptest/ptest.hpp"


bool test_manyEdSigning(int number_of_threads, size_t signs_num, size_t message_len);
bool test_user_sending ();
bool test_many_users ();

bool test_cheater();
bool test_fast_cheater();
bool test_malignant_cheater();
bool fast_find_cheater();

bool test_bad_chainsign();
bool test_convrt_tokenpacket();
bool test_netuser();
bool test_coinsign_error();
bool chrono_time();

bool test_wallet_expected_sender();
bool test_wallet_mint_check();
int test_mint_token_expiration();
bool test_recieve_deprecated_token();

//bool test_rpcwallet();

bool user_save_load();
bool netuser_save_load();

bool json_serialize();

bool test_all(int number_of_threads);

#endif // TESTS_HPP
