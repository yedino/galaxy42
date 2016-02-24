#include <iostream>
#include "tests.hpp"

bool check_cmd(const std::string cmd, const std::string request) {	//TODO return command id (int)
    int start_point = 0;
    while(start_point < request.size()) {
        if(request.at(start_point) != ' ') {
            break;
        }
        start_point++;		// avoiding spaces at the beggining of request
    }

    std::string subrequest = request.substr(start_point, request.find(' ', start_point));

    return (subrequest == std::string(cmd));
}

void run_interactive_protocol() {

    // SETTING
    std::string my_name;
    std::cout << "Set your name: ";
    std::getline(std::cin,my_name);

    int my_port;
    std::cout << "Set your port: ";
    std::cin >> my_port; std::cin.ignore();

    c_netuser A(my_name, my_port);

    std::string target_address;
    std::cout << "Enter target addres: ";
    std::getline(std::cin,target_address);

    int target_port;
    std::cout << "Enter target port: ";
    std::cin >> target_port; std::cin.ignore();

    bool isover = false;

    std::string help = "help info:\n";
                help += "emit - emit new token\n";
                help += "send - send token to target\n";
                help += "status - print status info\n";
                help += "quit - for close\n";
    std::cout << "for help type <help>" << std::endl;

    // MAIN LOOP
    while(!isover) {
        std::string request;
        std::cout << "Enter message: ";
        std::getline(std::cin,request);
        std::cout << std::endl;

        if(check_cmd("quit",request)) {
            isover = true;
            break;
        }
        else if(check_cmd("emit",request)) {
            A.emit_tokens(1);
        }
        else if(check_cmd("send",request)) {
            A.send_token_bynet(target_address, target_port);
        }
        else if(check_cmd("status",request)) {
            A.print_status(std::cout);
        }
        else if(check_cmd("help",request)) {
            std::cout << help << std::endl;
        }
        else {
            std::cout << "bad command! -- try again" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(0));
    }
}

int main (int argc, char *argv[]) {
	try {
		ios_base::sync_with_stdio(false);
        int number_of_threads;
        bool correct_threads_num = true;
		if (argc <= 1) {
            std::cout << "you could define number of theards in argv[1] to run test with" << std::endl;
            correct_threads_num = false;
        } else {
            number_of_threads = atoi(argv[1]);
        }
		if (number_of_threads <= 0) {
            std::cout << "you define incorrect number of theards" << std::endl;
            correct_threads_num = false;
        }
        if (!correct_threads_num) {
            std::cout << "setting default 1 thread" << std::endl;
            number_of_threads = 1;
        }
        //test_all(number_of_threads);

        run_interactive_protocol();

        return 0;

	} catch (std::exception& e) {
        std::cerr << "MainException: " << e.what() << std::endl;
	}
}
