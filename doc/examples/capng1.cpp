#include <iostream>
#include <string>
#include <linux/types.h>
#include <cap-ng.h>

using namespace std;

int main() {
	// as in https://people.redhat.com/sgrubb/libcap-ng/index.html
	cout << "Running, press key to drop" << endl;
	string s;
	getline(cin,s);


	capng_clear(CAPNG_SELECT_BOTH);
	capng_apply(CAPNG_SELECT_BOTH);
	cout << "Dropped." << endl;


	cout << "Press enter to exit" << endl;
	getline(cin,s);
}

