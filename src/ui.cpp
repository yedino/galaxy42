
#include "ui.hpp"

using namespace std;

namespace ui {



void action_error_exit(const string &info) {
	cout << info << endl << "Program will now exit because of above reason." << endl;
	throw exception_error_exit();
}

void action_info_ok(const string &info) {
	cout << "OK: " << info << endl;
}




}


