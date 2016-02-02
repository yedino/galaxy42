#ifndef TESTS_HPP
#define TESTS_HPP

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "c_netuser.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"

#include "ptest/ptest.hpp"




bool test_readableEd ();
bool test_manyEdSigning(int number_of_threads, size_t signs_num, size_t message_len);
bool test_user_sending ();
bool test_many_users ();
bool test_cheater();
bool test_bad_chainsign();
bool test_convrt_tokenpacket();

bool test_netuser();

bool test_all(int number_of_threads);




#endif // TESTS_HPP
