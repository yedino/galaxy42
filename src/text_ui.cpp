// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "text_ui.hpp"
#include <boost/locale.hpp>
#include <string>
#include <libintl.h>
#include <locale.h>

bool text_ui::ask_user_forpermission(const std::string &msg) {
	std::string answer = "";
//	std::cout << "Do you want : " << msg << " -- (YES/no):";
        std::cout << boost::locale::gettext("L_do_you_want")  << msg << boost::locale::gettext( "L_Y_N") ;

	std::getline(std::cin,answer);
	if(answer == "YES") {
		return true;
	}
	return false;
}
