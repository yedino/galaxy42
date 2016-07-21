// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "text_ui.hpp"
#include <string>

bool text_ui::ask_user_forpermission(const std::string &msg) {
	std::string answer = "";
	std::cout << "Do you want : " << msg << " -- (YES/no):";
	std::getline(std::cin,answer);
	if(answer == "YES") {
		return true;
	}
	return false;
}
