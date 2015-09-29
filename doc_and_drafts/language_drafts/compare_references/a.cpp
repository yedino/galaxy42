#include <iostream>
using namespace std;

class c_test {
	public:
		double aaa, bbb;
};

int main() {
	c_test foo, bar;

	c_test & r1 = foo;
	c_test & r2 = foo;
	c_test & r3 = bar;

	bool same1 = ( & r1 == & r2);
	bool same2 = ( & r1 == & r3);

	cout << same1 << " " << same2 << endl;


}


