#include <iostream>
#include <list>
#include "c_user.hpp"

using namespace std;

bool test_user_sending() {

    c_user test_userA;
    c_user test_userB;

    test_userA.emit_tokens(1);
    test_userA.send_token(test_userB,1);
    //test_userA.send_token(test_userB,1);

    return 0;
}

int main () {

    test_user_sending();
	return 0;
}
