#include "text_ui.hpp"


bool text_ui::ask_user_forpermission(const std::string &msg) {
	std::string answer = "";
	std::cout << "Do you want : " << msg << " -- (YES/no):";
	std::getline(std::cin,answer);
	if(answer == "YES") {
		return true;
	}
	return false;
}
