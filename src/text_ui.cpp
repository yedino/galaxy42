// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "text_ui.hpp"

#include <string>
#include <locale>
#include "mo_reader.hpp"

bool text_ui::ask_user_forpermission(const std::string &msg) {
	std::string answer = "";
//	std::cout << "Do you want : " << msg << " -- (YES/no):";
        std::cout << mo_file_reader::gettext("L_do_you_want")  << msg << mo_file_reader::gettext( "L_Y_N") ;

	std::getline(std::cin,answer);
	if(answer == "YES") {
		return true;
	}
	return false;
}
