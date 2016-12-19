// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "ui.hpp"
#include <boost/locale.hpp>


using namespace std;

namespace ui {


exception_error_exit::exception_error_exit(const std::string &what)
	: m_what(what)
{ }

const char* exception_error_exit::what() const { return m_what.c_str(); }



void action_error_exit(const string &info) {
//	cout << info << endl << "Program will now exit because of above reason." << endl;
        cout << info << endl << mo_file_reader::gettext("L_exit_from_connection_attempt") << endl;

	throw exception_error_exit(info);
}

void action_info_ok(const string &info) {
	_note("OK: " << info);
	cout << "OK: " << info << endl;
}

void show_error_exception_show_what(const std::string &during_action, const std::string &what) {
//        std::cerr<<"Error occured during '"<<during_action<<"' - " << what << std::endl;
	std::cerr<<mo_file_reader::gettext("L_error_during_action")<<during_action<<"' - " << what << std::endl;
}

void show_error_exception(const string &during_action, const std::exception &e) { ///< Inform user about caught exception
//	_info("Exception caught: during_action=["<<during_action<<"], what="<<e.what());
        _info(mo_file_reader::gettext("L_exception_during_action")<<during_action<<mo_file_reader::gettext("L_what_exception")<<e.what());

	string what = e.what();
//	if (!what.size()) what="(No additional information about this exception, it was empty)";
        if (!what.size()) what=mo_file_reader::gettext("L_no_additional_info_exception");

	//	else what = "Exception: " + what;
        else what = mo_file_reader::gettext("L_exception") + what;

	show_error_exception_show_what(during_action, what);
}

void show_error_exception_unknown(const string &during_action) { ///< Inform user about caught exception of unknown type
//	_info("Exception caught: during_action=["<<during_action<<"] " << "(Type of exception is unknown)");
        _info(mo_file_reader::gettext("L_exception_during_action")<<during_action<<"] " << mo_file_reader::gettext("L_unknown_exception"));

//	show_error_exception_show_what(during_action, "(Type of exception is unknown)");
        show_error_exception_show_what(during_action, mo_file_reader::gettext("L_unknown_exception"));

}


} // namespace


