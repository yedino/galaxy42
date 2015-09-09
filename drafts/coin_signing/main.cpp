#include <iostream>
#include <list>
#include "c_user.hpp"
#include "c_encryption.hpp"

using namespace std;

bool test_readableEd () {
    std::cout << "RUNNING TEST00 READABLE_CRYPTO_ED" << std::endl;
    c_ed25519 edtest;
    std::string str_pubkey = edtest.get_public_key();
    unique_ptr<unsigned char[]> utab_pubkey = edtest.get_public_key_C();
    std::cout << "ed: original utab pubkey = " << utab_pubkey.get() << std::endl;
    std::cout << "ed: string format pubkey = " << str_pubkey << std::endl;
    unique_ptr<unsigned char[]> utab_pubkey_afttrans (readable_toUchar(str_pubkey, pub_key_size));
    std::cout << "ed: back to short format = " << utab_pubkey_afttrans.get() << std::endl;

    const std::string message = "3fd30542fe3f61b24bd3a4b2dc0b6fb37fa6f63ebce52dd1778baa8c4dc02cff";
    const int message_len = message.length();
    std::string sign = edtest.sign(message);

    /* verify the signature */
    if (edtest.verify(sign, message, message_len, str_pubkey)) {
        std::cout << "valid signature\n" << std::endl;
    } else {
        std::cout << "invalid signature\n" << std::endl;
        return true;
    }

    return false;
}

bool test_user_sending () {
    std::cout << "RUNNING TEST01 USER_SENDING" << std::endl;
	c_user test_userA("userA");
	c_user test_userB("userB");

	test_userA.emit_tokens(1);
	test_userA.send_token(test_userB, 1);

    return false;
}

bool test_many_users () {
    std::cout << "RUNNING TEST02 MANY_USER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), D("userD");

	A.emit_tokens(1);
	A.send_token(B);
	B.send_token(C);
	C.send_token(D);
	D.send_token(A);
	return false;
}

bool test_cheater() {

    std::cout << "RUNNING TEST03 CHEATER" << std::endl;
    c_user A("userA"), B("userB"), C("userC"), X("userX");
    A.emit_tokens(1);
    A.send_token(B);
    B.send_fake_token(C);
    B.send_token(X);
    C.send_token(A);
    X.send_token(A); // should detect cheater
    return false;
}

int main () {

    test_readableEd();
    test_user_sending();
    test_many_users();
    test_cheater();
	return 0;
}
