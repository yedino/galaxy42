// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "ui.hpp"

using namespace std;

namespace ui {



void action_error_exit(const string &info) {
	cout << info << endl << "Program will now exit because of above reason." << endl;
	throw exception_error_exit();
}

void action_info_ok(const string &info) {
	_note("OK: " << info);
	cout << "OK: " << info << endl;
}




}


