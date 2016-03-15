#include "tests.hpp"

new_test_suite(many_ed_signing);
new_test_suite(base_tests);
new_test_suite(cheater_tests);
//new_test_suite(bitwallet);
new_test_suite(wallet_io);

using std::thread;
using std::mutex;
using std::atomic;

atomic<size_t> tests_counter(0);
mutex mtx;


bool test_all(int number_of_threads) {

    ptest::config_t config_default;
    std::fstream file("./test.log", std::ios_base::out | std::ios_base::trunc);
    config_default.set_all_output_to(file);
    config_default.print_passed_tests = true;

    many_ed_signing.config = config_default;
//    base_tests.config = config_default;
//    cheater_tests.config = config_default;
//    bitwallet.config = config_default;
//    wallet_io.config = config_default;

    //ptest::general_suite.config = config_default;

    int test_loop_num = 100, msg_length = 64;

    ptest::call_test(number_of_threads,
                            [&number_of_threads, &test_loop_num, &msg_length] () {
                                run_suite_test(many_ed_signing,test_manyEdSigning, number_of_threads, test_loop_num, msg_length, false, pequal);
                            }
                    );

    run_suite_test(base_tests, test_user_sending , 0, pequal);
    run_suite_test(base_tests, test_many_users , 0, pequal);

    run_suite_test(cheater_tests, test_cheater , 0, pequal);
    run_suite_test(cheater_tests, test_fast_cheater , 0, pequal);
    run_suite_test(cheater_tests, test_malignant_cheater , 0, pequal);
    run_suite_test(cheater_tests, fast_find_cheater , 0, pequal);

    run_suite_test(base_tests, test_bad_chainsign, 0, pequal);
    run_suite_test(base_tests, test_convrt_tokenpacket, 0, pequal);
    run_suite_test(base_tests, test_netuser, 0, pequal);
    run_suite_test(base_tests, test_coinsign_error, 0, pequal);
    run_suite_test(base_tests, chrono_time, 0, pequal);

    run_suite_test(wallet_io, test_wallet_expected_sender, 0, pequal);
    run_suite_test(wallet_io, test_wallet_mint_check, 0, pequal);
    run_suite_test(wallet_io, test_mint_token_expiration, 10, pequal);
    run_suite_test(wallet_io, test_recieve_deprecated_token, 0, pequal);

    run_suite_test(base_tests, user_save_load, 0, pequal);
    run_suite_test(base_tests, netuser_save_load, 0, pequal);
    // To pass below test. Running ./bitcoind or ./bitccoin-qt on your mashine is required
    //run_suite_test(bitwallet,test_rpcwallet, 0, pequal);

    run_suite_test(base_tests, json_serialize, 0, pequal);

    print_final_suite_result(many_ed_signing);

    print_final_suite_result(base_tests);
    print_final_suite_result(cheater_tests);
    print_final_suite_result(wallet_io);
    //print_final_suite_result(bitwallet);

    print_final_test_result();
    return false;
}

// this test using old ed25519 wrapper - DEPRECATED
// now readable ed we getting by << operator on ustring wrapper
//bool test_readableEd () {
//    std::cout << "RUNNING TEST READABLE_CRYPTO_ED" << std::endl;
//    c_ed25519 edtest;
//    std::string str_pubkey = edtest.get_public_key();
//    unique_ptr<unsigned char[]> utab_pubkey = edtest.get_public_key_uC();
//    std::cout << "ed: original utab pubkey = " << utab_pubkey.get() << std::endl;
//    std::cout << "ed: string format pubkey = " << str_pubkey << std::endl;
//    unique_ptr<unsigned char[]> utab_pubkey_afttrans (readable_toUchar(str_pubkey, pub_key_size));
//    std::cout << "ed: back to short format = " << utab_pubkey_afttrans.get() << std::endl;

//    const std::string message = "3fd30542fe3f61b24bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
//    std::string sign = edtest.sign(message);

//    /* verify the signature */
//    if (edtest.verify(sign, message, str_pubkey)) {
//        std::cout << "valid signature\n" << std::endl;
//    } else {
//        std::cout << "invalid signature\n" << std::endl;
//        return true;
//    }

//    return false;
//}



bool test_manyEdSigning(int number_of_threads, size_t signs_num, size_t message_len) {

    crypto_ed25519::keypair keys(crypto_ed25519::generate_key());

    for(size_t i = 1; i < signs_num; ++i) {
        const std::string message = cs_utils::generate_random_string(message_len);
        ed_sign sign = crypto_ed25519::sign(message, keys);

        /* verify the signature */
        if (run_suite_assert (many_ed_signing, crypto_ed25519::verify_signature(message, sign, keys.public_key) == true, "invalid signature!") == pfailed) return true;

        mtx.lock();
        if( !(i % ((signs_num*number_of_threads)/100))) {
            tests_counter++;

        std::cout << "[" << tests_counter << "%]"
                  << "\b\b\b\b\b\b" << std::flush;
        }
        mtx.unlock();
    }
    return false;
}

bool test_user_sending () {
    std::cout << "RUNNING TEST USER_SENDING" << std::endl;
    c_user test_userA("userA");
    c_user test_userB("userB");

    test_userA.emit_tokens(1);
    test_userA.send_token_bymethod(test_userB);

    return false;
}

bool test_many_users () {
    std::cout << "RUNNING TEST02 MANY_USER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD");

    A.emit_tokens(1);
    A.send_token_bymethod(B);
    B.send_token_bymethod(C);
    C.send_token_bymethod(D);
    D.send_token_bymethod(A);
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// cheater tests

bool test_cheater() {
  try {
    std::cout << "RUNNING TEST03 CHEATER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD"), X("userX");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(C,1);
    C.send_token_bymethod(D);
    D.send_token_bymethod(X);
    X.send_token_bymethod(A);

    B.send_token_bymethod(D); // now cheater will be detect here
    D.send_token_bymethod(B);
    B.send_token_bymethod(X);
    X.send_token_bymethod(A); // should detect cheater

    A.print_status(std::cout);
  } catch (coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 15) {
            return false;
        }
  }
    return true;
}

bool test_fast_cheater() {

  try {
    std::cout << "RUNNING TEST03 CHEATER" << std::endl;
    c_user A("userA"), B("userB");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(A,1);
    B.send_token_bymethod(A); // should detect cheater B

    A.print_status(std::cout);
  } catch (coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 15) {
            return false;
        }
  }
    return true;
}


bool test_malignant_cheater() {
  try {
    std::cout << "RUNNING TEST03b MALIGNANT_CHEATER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD"), X("userX");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(C,1);
    B.send_token_bymethod(C);
    C.send_token_bymethod(A);	// user C should detect the cheater using simple malignant test
    C.send_token_bymethod(A);

    A.print_status(std::cout);
  } catch (coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 15) {
            return false;
        }
  }
    return true;
}

bool fast_find_cheater() {
  try {
    std::cout << "RUNNING FAST_FIND_CHEATER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(C,1);
    B.send_token_bymethod(D);
    C.send_token_bymethod(A);
    D.send_token_bymethod(C);	// detecting cheater by user

  } catch (coinsign_error &cec) {
        //std::cout << cec.what() << std::endl;
        if(cec.get_code() == 15) {
            return false;
        }
  }
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// cheater test end

bool test_bad_chainsign() {
    std::cout << "RUNNING BAD INPUT, CHAINSIGN FORMAT TEST" << std::endl;
    int errors = 3;
 try{
    c_token("$0|ed5c06c0577e0bf8a22e5f2c09f471e2c288bedfe255882d265661bf382e784232"
            "&ed1efe422eb7e56eb77d86efdac68b24da0ec4667ec44e45cd9db54389effb533"
            "a2a7f26496e630e8e0877eb96dae7a6d2ee2b6aaa84a0a755f645239597f25e02"
            "&userA&eda1da39fb7fe9b34013f26b20a6b6550c31ef4452c8f2602b5be19be289840204"
            "$12|ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea"
            "&ed61ccfae983e6660505443ef89e1749c1e9007920793c7c935737822d26c8e70"
            "ba433fa25bb19abf5e3d4fe57f466ea001711d369d4505a10d868278db86b200b"
            "&userB&ed5c06c0577e0bf8a22e5f2c09f471e2c288bedfe255882d265661bf382e784232"
            "$234|edb1fdafc96abac26c52dae3be8b68e3b8e479ebdcc3dbf97e81bf1f47d0472bcf"
            "&edbde6d2088a2a3520ff5e0222cbb18465e0e3f2b39ca4b45a7ae97b3e56e6909"
            "7783d0b2c54e2d9f12f7d7d5fdd4c80d936e1b7e7006cad4eaf731133a935c20a"
            "&userC&ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea", 2); // bad ids
 } catch(std::exception &message) {
    std::cout << message.what() << " -- OK" << std::endl;
    errors--;
 }
 try {
    c_token("$|ed5c06c0577e0bf8a22e5f2c09f471e2c288bedfe255882d265661bf382e784232"
            "&ed1efe422eb7e56eb77d86efdac68b24da0ec4667ec44e45cd9db54389effb533"
            "a2a7f26496e630e8e0877eb96dae7a6d2ee2b6aaa84a0a755f645239597f25e02"
            "&userA&eda1da39fb7fe9b34013f26b20a6b6550c31ef4452c8f2602b5be19be289840204"
            "$ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea"
            "&ed61ccfae983e6660505443ef89e1749c1e9007920793c7c935737822d26c8e70"
            "ba433fa25bb19abf5e3d4fe57f466ea001711d369d4505a10d868278db86b200b"
            "&userB&ed5c06c0577e0bf8a22e5f2c09f471e2c288bedfe255882d265661bf382e784232"
            "$0|edb1fdafc96abac26c52dae3be8b68e3b8e479ebdcc3dbf97e81bf1f47d0472bcf"
            "&edbde6d2088a2a3520ff5e0222cbb18465e0e3f2b39ca4b45a7ae97b3e56e6909"
            "7783d0b2c54e2d9f12f7d7d5fdd4c80d936e1b7e7006cad4eaf731133a935c20a"
            "&userC&ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea", 2); // no ids
 } catch(std::exception &message) {
    std::cout << message.what() << " -- OK" << std::endl;
    errors--;
 }
 try{
    c_token("$23|aaaaa$123|sbbbbbf#pass", 2);	// bad format
 } catch(std::exception &message) {
    std::cout << message.what() << " -- OK" << std::endl;
    errors--;
 }
    std::cout << "Errors not passed: " << errors << std::endl;
    if(errors == 0) {
        return false;
    } else {
        return true;
    }
}

bool test_convrt_tokenpacket() {
 try {
    std::cout << "RUNNING TEST04 CONVERTING BETWEEN TOKEN<-->PACKET" << std::endl;

    c_user A("userA"), B("userB"), C("userC"), D("userD");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(C);
    C.send_token_bymethod(D);

    std::string packet = D.get_token_packet(2, A.get_public_key(), 1);
    c_token test_tok(packet, 2);
    std::string packet_two = test_tok.to_packet(2);

    if(packet == packet_two) {
        return false;
    }
    else {
        std::cout << "packet1:/n" << packet << std::endl;
        std::cout << "packet2:/n" << packet_two << std::endl;
        std::string error("error: after converting token<->packet tokens doesn't match");
        throw std::logic_error(error);
    }

 } catch(std::exception &ec) {
    std::cout << ec.what() << std::endl;
    return true; // error
 }
}

bool test_netuser() {
  try {
    std::cout << "RUNNING_NETUSER_TEST" << std::endl;
    std::string userA_name("testUser1");
    std::string userB_name("testUser2");

    c_netuser A(userA_name, 30000);
    c_netuser B(userB_name, 30001);

    A.emit_tokens(2);
    A.send_token_bynet("::1", 30001);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    A.print_status(std::cout);
    B.print_status(std::cout);

    B.get_token_packet(2, A.get_public_key());	// is wallet empty?
    return false;
  } catch(std::exception &ec) {
        std::cout << ec.what() << std::endl;
        return true;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////// bitwallet test

//bool test_rpcwallet() {
//  try {
//    std::cout << "RUNNING_RPCWALLET_TEST" << std::endl;
//    c_user BitUser("namecoin_user");
//    if (run_suite_assert (bitwallet,BitUser.check_bitwallet() == false, "wallet should be unset here!") == pfailed) return true;

//    /***
//     * I use my own Xcoin client for this test
//     * If you want to correct use bitwallet look at your .Xcoin/Xcoin.conf for:
//     * 		rpcuser
//     *      rpcpassword
//     *		rpcport
//     *
//     * Be sure you have Xcoind running before start using bitwallet
//     */

//    std::string rpc_usr = "nmc_testadmin";
//    std::string rpc_passwd = "dontworrybehappy";
//    std::string rpc_host = "127.0.0.1";
//    int rpc_port = 8336;

//    BitUser.set_bitwallet(rpc_usr,rpc_passwd,rpc_host,rpc_port);

//    if (run_suite_assert (bitwallet,BitUser.check_bitwallet() == true, "wallet should be correctly set here!") == pfailed) return true;

//    std::cout << "Your namecoin balance is: " << std::fixed << std::setprecision(8) <<
//                 BitUser.get_bitwallet_balance() << std::endl;
//    return false;
//  } catch(BitcoinException &btc_ec) {
//        std::cout << btc_ec.getCode() << ": " << btc_ec.getMessage() << std::endl;
//        std::cout << btc_ec.what() << std::endl;
//        return true;
//  }
//}

//////////////////////////////////////////////////////////////////////////////////////////////////// wallet_io

bool test_wallet_expected_sender() {
  try {
    std::cout << "RUNNING_WALLET_EXPECTED_SENDER_TEST" << std::endl;

    std::string userA_name("MintWallet");
    std::string userB_name("thiefUser");
    std::string userC_name("WalletUser02");
    c_user A(userA_name);
    c_user B(userB_name);
    c_user C(userC_name);
    A.emit_tokens(2);

    A.send_token_bymethod(C);

    C.save_coinwallet("./wallet.dat");
    B.load_coinwallet("./wallet.dat");

    C.send_token_bymethod(A);
    B.send_token_bymethod(A); 	// should detect bad expected sender
  } catch(coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 10) {
            return false;
        }
  }
    return true;
}

bool test_wallet_mint_check() {
  try {
    std::cout << "RUNNING_WALLET_MINT_CHECK_TEST" << std::endl;

    std::string userA_name("MintWallet");
    std::string userB_name("thiefUser");
    c_user A(userA_name);
    c_user B(userB_name);
    A.emit_tokens(2);

    A.print_status(std::cout);

    A.save_coinwallet("./wallet.dat");
    B.load_coinwallet("./wallet.dat");

    // should detect stolen wallet database
    B.send_token_bymethod(A);

  } catch(coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 13){
            return false;
        }
  }
    return true;
}

int test_mint_token_expiration() {
  try {
    std::cout << "RUNNING_WALLET_MINT_CHECK_TEST" << std::endl;

    c_user A("A_user");
    A.set_new_mint("fast_tokens", A.get_public_key(), std::chrono::seconds(2));

    int token_to_emit = 5;
    A.emit_tokens(token_to_emit);
    A.print_status(std::cout);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    int expiried_tokens = A.tokens_refresh();
    A.print_status(std::cout);		// now mint and wallet should be empty
    if (run_suite_assert (wallet_io, A.get_mint_last_expired_id() == token_to_emit ,
                          "Last id should be different!") == pfailed) {
        return true;
    }
    return expiried_tokens;

  } catch(std::exception &ec) {
        std::cerr << ec.what() << std::endl;
        return true;	// test expect return 10 (5 copy in mint + 5 in wallet)
  }
}

bool test_recieve_deprecated_token() {
  try {
    std::cout << "RUNNING_RECIEVE_DEPRECATED_TOKEN" << std::endl;

    c_user A("A_mint_user");
    c_user B("B_slow_user");
    A.set_new_mint("fast_tokens", A.get_public_key(), std::chrono::seconds(4));

    int token_to_emit = 1;
    A.emit_tokens(token_to_emit);
    A.print_status(std::cout);

    A.send_token_bymethod(B);

    std::this_thread::sleep_for(std::chrono::seconds(4));
    A.print_status(std::cout);		// now mint and wallet should be empty

    B.send_token_bymethod(A);		// deprecated token

  } catch(coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 12){
            return false;
        }
  }
    return true;
}

bool test_coinsign_error() {
  try {
    throw(coinsign_error(1,"error"));
  } catch(coinsign_error &cec) {
        std::cout << cec.what() << std::endl;
        if(cec.get_code() == 1) {
            return 0;
        }
  }
    return 1;
}

bool chrono_time() {
  try {
    std::cout << "RUNNING TEST_CHRONO_TIME" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD");
    A.emit_tokens(3);
    std::chrono::milliseconds all(0);
    for(int i = 0; i < 3; ++i) {
        A.send_token_bymethod(B);
        for(int j = 0; j < 10; ++j) {
            B.send_token_bymethod(C);
            C.send_token_bymethod(D);
            D.send_token_bymethod(C);
            C.send_token_bymethod(B);
        }
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        B.send_token_bymethod(A);
        std::chrono::time_point<std::chrono::system_clock> after = std::chrono::system_clock::now();
        all += std::chrono::duration_cast<std::chrono::milliseconds>(after - now);
    }
    std::cout << "recieving size = 100 token: "<< all.count()/5 << " miliseconds" << std::endl;

  } catch(std::exception &ec){
        std::cout << ec.what() << std::endl;
        return true;
  }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// user save --> load

bool user_save_load() {
  try {
    std::cout << "RUNNING TEST USER SAVE LOAD" << std::endl;

    c_user test_user("test_user"), other_user("other_user");
    test_user.emit_tokens(5);
    other_user.emit_tokens(1);
    other_user.send_token_bymethod(test_user);
    test_user.save_user();
    test_user.print_status(std::cout);

    c_user loaded("test_user2");
    loaded.load_user("test_user.dat");
    loaded.save_user("test_user2.dat");
    loaded.print_status(std::cout);

    std::ifstream ofs01("test_user.dat");
    std::ifstream ofs02("test_user2.dat");
    std::string file01; ofs01 >> file01;
    std::string file02; ofs02 >> file02;


    if(file01 == file02) {
        std::cout << "Flies are equal - OK" << std::endl;
        return false;
    }

  } catch(std::exception &ec){
        std::cout << ec.what() << std::endl;
        return true;
  }
    return true;
}

bool netuser_save_load() {
  try {
    std::cout << "RUNNING TEST NETUSER SAVE LOAD" << std::endl;

    c_user test_user("test_user"), other_user("other_user");
    test_user.emit_tokens(5);
    other_user.emit_tokens(1);
    other_user.send_token_bymethod(test_user);
    test_user.print_status(std::cout);

    {
        c_netuser test_netuser(std::move(test_user));
        test_netuser.save_user();
        test_netuser.print_status(std::cout);
    }

    c_netuser loaded("test_user2");
    loaded.load_user("test_user.dat");
    loaded.save_user();
    loaded.print_status(std::cout);

    std::ifstream ofs01("test_user.dat");
    std::ifstream ofs02("test_user2.dat");
    std::string file01; ofs01 >> file01;
    std::string file02; ofs02 >> file02;


    if(file01 == file02) {
        std::cout << "Flies are equal - OK" << std::endl;
        return false;
    }

  } catch(std::exception &ec){
        std::cout << ec.what() << std::endl;
        return true;
  }
    return true;
}

bool json_serialize() {
  try {
        std::cout << "RUNNING TEST JSON SERIALIZE" << std::endl;

        c_user A("userA"), B("userB"), C("userC"), D("userD");
        A.emit_tokens(1);

        A.send_token_bymethod(B);
        B.send_token_bymethod(C);
        C.send_token_bymethod(D);

        // boost::serialize way
        std::string packet = D.get_token_packet(2, A.get_public_key(),1);
        c_token tok_serialize(packet, 2);

        // json::value way
        std::string output;
        c_json_serializer::serialize(&tok_serialize, output);

        std::cout << "TEST JSON: " << output << std::endl;

        c_token tok_deserialize;
        c_json_serializer::deserialize(&tok_deserialize, output);

        tok_serialize.print(std::cout,true);
        tok_deserialize.print(std::cout,true);

        //std::cout << "Static_cast :" << static_cast<size_t>(-1) << std::endl;
        //std::cout << "Maximum value numeric_cast :" << std::numeric_limits<size_t>::max() << std::endl;

        if(tok_serialize == tok_deserialize) {
            std::cout << "Json serialization SUCCESS" << std::endl;
            return false;
        }

  } catch(std::exception &ec){
        std::cout << ec.what() << std::endl;
        return true;
  }
    return true;
}
