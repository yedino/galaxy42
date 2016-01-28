#include <iostream>
#include <list>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include "c_netuser.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"

using std::thread;
using std::mutex;
using std::atomic;

int number_of_threads;
atomic<size_t> tests_counter(0);
mutex mtx;

string generate_random_string (size_t length) {
	auto generate_random_char = [] () -> char {
		const char Charset[] = "0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(Charset) - 1);
		return Charset[rand() % max_index];
	};
	string str(length, 0);
	generate_n(str.begin(), length, generate_random_char);
	return str;
}


bool test_readableEd () {
	std::cout << "RUNNING TEST READABLE_CRYPTO_ED" << std::endl;
	c_ed25519 edtest;
	std::string str_pubkey = edtest.get_public_key();
	unique_ptr<unsigned char[]> utab_pubkey = edtest.get_public_key_uC();
	std::cout << "ed: original utab pubkey = " << utab_pubkey.get() << std::endl;
	std::cout << "ed: string format pubkey = " << str_pubkey << std::endl;
	unique_ptr<unsigned char[]> utab_pubkey_afttrans (readable_toUchar(str_pubkey, pub_key_size));
	std::cout << "ed: back to short format = " << utab_pubkey_afttrans.get() << std::endl;

	const std::string message = "3fd30542fe3f61b24bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
	std::string sign = edtest.sign(message);

	/* verify the signature */
	if (edtest.verify(sign, message, str_pubkey)) {
		std::cout << "valid signature\n" << std::endl;
	} else {
		std::cout << "invalid signature\n" << std::endl;
		return true;
	}

	return false;
}

bool test_manyEdSigning(size_t signs_num, size_t message_len) {
	c_ed25519 edtest;
	std::string str_pubkey = edtest.get_public_key();

	for(size_t i = 1; i < signs_num; ++i) {
		const std::string message = generate_random_string(message_len);
		std::string sign = edtest.sign(message);

		/* verify the signature */
		if (!edtest.verify(sign, message, str_pubkey)) {
			std::cout << "\ninvalid signature!\n" << std::endl;
			return true;
		}
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

bool test_cheater() {

	std::cout << "RUNNING TEST03 CHEATER" << std::endl;
	c_user A("userA"), B("userB"), C("userC"), X("userX");
    A.emit_tokens(1);


    A.send_token_bymethod(B);
    B.send_token_bymethod(C,1);
    B.send_token_bymethod(X);
    X.send_token_bymethod(B);
    B.send_token_bymethod(X);
    X.send_token_bymethod(B);
    B.send_token_bymethod(A);
    C.send_token_bymethod(X);
    X.send_token_bymethod(A); // should detect cheater

    A.emit_tokens(2);
    B.emit_tokens(1);
    B.send_token_bymethod(A);
    A.print_status(std::cout);

    return false;
}

bool test_bad_chainsign() {
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
            "&userC&ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea"); // bad ids
 } catch(std::string &message) {
    std::cout << message << " -- OK" << std::endl;
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
            "&userC&ede83da978ac24294ad3c4ce1a14a184dcaf1ce2d35551242986efb7dd4cca68ea"); // no ids
 } catch(std::string &message) {
    std::cout << message << " -- OK" << std::endl;
    errors--;
 }
 try{
    c_token("$23|aaaaa$123|sbbbbbf#pass");	// bad format
 } catch(std::string &message) {
    std::cout << message << " -- OK" << std::endl;
    errors--;
 }
    if(errors == 0) {
        return false;
    } else {
        return true;
    }
}

bool test_convrt_tokenpacket() {

    std::cout << "RUNNING TEST04 CONVERTING BETWEEN TOKEN<-->PACKET" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD");
    A.emit_tokens(1);

    A.send_token_bymethod(B);
    B.send_token_bymethod(C);
    C.send_token_bymethod(D);
    D.send_token_bymethod(A);


 try {
    std::string packet = D.get_token_packet(A.get_public_key());
    c_token test_tok(packet);
    std::string packet_two = test_tok.to_packet();

    if(packet == packet_two) {
        return false;
    }
    else {
        throw(std::string("error: after converting token<->packet tokens doesn't match"));
    }

 } catch(std::string &message) {
    std::cerr << message << std::endl;
    return true; // error
 }

    return false;
}

bool test_netuser() {
	std::string userA_name("userA");
	std::string userB_name("userB");
    c_netuser A(userA_name, 30000);
	c_netuser B(userB_name, 30001);
    //bool isover = false;

	A.emit_tokens(3);
	//A.print_status(std::cout);
	A.send_token_bynet("127.0.0.1", 30001);
	//B.print_status(std::cout);

	B.send_token_bynet("127.0.0.1", 30000);
	//A.print_status(std::cout);


    //robert
    //std::string target_ip_addr = "192.168.0.57";
    //std::string target_pubkey = "ed5ea262180d88dd8ea31a458358513d52819d3fa09e541e2f96a5e5858920293e";
    //my
    /*std::string target_ip_addr = "127.0.0.1";
    std::string target_pubkey = A.get_public_key();

    std::cout << "Your target is " << target_ip_addr << " with pubkey " << target_pubkey << std::endl;

    std::cout << "For exit type <quit>\n";
    std::cout << "emit token <emit>\n";
    std::cout << "send token <send>\n";
    while(!isover) {
        std::cout << "Enter message: ";
        std::string request;
        std::getline(std::cin,request);

        if(request == "quit") {
            isover = true;
            break;
        }
        else if(request == "emit") {
            A.emit_tokens(1);
        }
        else if(request == "send") {
            A.send_token_bynet(target_ip_addr);
        }
        else {
            std::cout << "bad command! -- try again" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
*/
    return 0;
}


bool test_all() {

	list<thread> Threads;

    int test_loop = 100, msg_length = 64;
	std::cout << "RUNNING TEST MANY_ED_SIGNING IN " << number_of_threads << " THREADS" << std::endl;
	for (int i = 0; i < number_of_threads; ++i) {
		Threads.emplace_back([&test_loop, &msg_length](){test_manyEdSigning(test_loop,msg_length);});
	}

	for (auto &t : Threads) {
		t.join();
	}

    if(   //	!test_readableEd() &&
          //  !test_user_sending() &&
          //  !test_many_users() &&
          //  !test_cheater() &&
          //  !test_bad_chainsign() &&
          //  !test_convrt_tokenpacket() &&
            !test_netuser())  {
		return 0;
	} else {
		return 1;
	}
}

int main (int argc, char *argv[]) {
	try {
		ios_base::sync_with_stdio(false);

		if (argc <= 1) {
			cout << "please define number of theards to run test\n";
			return 0;
		}

		number_of_threads = atoi(argv[1]);

		if (number_of_threads <= 0) {
			cout << "please define correct number of theards to run test\n";
			return 1;
		}

		test_all();

		return 0;

	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

