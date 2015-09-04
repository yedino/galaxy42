#include <iostream>
#include <list>
#include "c_user.hpp"

using namespace std;

bool test_user_sending () {

	c_user test_userA;
	c_user test_userB;

	test_userA.emit_tokens(1);
	test_userA.send_token(test_userB, 1);

	return 0;
}

bool test_many_users () {

	c_user A, B, C, D;

	A.emit_tokens(1);
	A.send_token(B);
	B.send_token(C);
	C.send_token(D);
	D.send_token(A);
}

int main () {

	test_user_sending();
	test_many_users();
	return 0;
}
