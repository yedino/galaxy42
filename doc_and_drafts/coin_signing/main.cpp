#include "libs01.hpp"
#include "tests.hpp"


int check_cmd(const std::string request) {
    size_t start_point = 0;
    while(start_point < request.size()) {
        if(request.at(start_point) != ' ') {
            break;
        }
        start_point++;		// avoiding spaces at the beggining of request
    }
    std::string subrequest = request.substr(start_point, request.find(' ', start_point));

    // COMMANDS
    if(subrequest == "quit") {
        return 0;
    } else if(subrequest == "help") {
        return 1;
    } else if(subrequest == "emit") {
        return 2;
    } else if(subrequest == "send") {
        return 3;
    } else if(subrequest == "status") {
        return 4;
    } else if(subrequest == "save") {
        return 5;
    } else if(subrequest == "load") {
        return 6;
    } else if(subrequest == "target") {
        return 7;
    } else {
        return -1;
    }
}

void set_interactive_target(std::string &address, int &port) {

    std::cout << "Enter target addres: ";
    std::getline(std::cin,address);
    std::cout << "Enter target port: ";
    std::cin >> port; std::cin.ignore();
}

void run_interactive_protocol() {

    char decision = 'n';
    bool new_user = false;
    std::cout << "Do you want to create a new user? (Y/n): ";
    std::cin >> decision; std::cin.ignore();
    if(decision == 'Y' || decision == 'y') {
        new_user = true;
    }
    // SETTING
    int my_port;
    std::cout << "Set your server port for this session: ";
    std::cin >> my_port; std::cin.ignore();

    std::string my_name = "default";
    std::string filename = "user.dat";
    c_netuser A(my_name,my_port);

    if(new_user) {
        std::cout << "Set your new username: ";
        std::getline(std::cin,my_name);
        A.set_username(my_name);
        A.set_new_mint(my_name+"_mint",A.get_public_key());
    } else {
        if(A.load_user(filename)) {
            std::cout << "Successfully loaded user: " << A.get_username() << std::endl;
        } else {
            std::cout << "Fail to load user: exiting";
            return;
        }
    }


    std::string target_address;
    int target_port;
    decision = 'n';

    std::cout << "Do you want to set terget now? (Y/n): ";
    std::cin >> decision; std::cin.ignore();
    if(decision == 'Y' || decision == 'y') {
        set_interactive_target(target_address,target_port);
    }

    // HELP
    std::string help = "help info:\n";
                help += "emit - emit new token\n";
                help += "send - send token to target\n";
                help += "status - print status info\n";
                help += "save - save user status (default in user.dat)\n";
                help += "load - load your last saved user status\n"
                        "       attention, you could lost this session changes\n";
                help += "target - setting new terget address and server port\n";
                help += "quit - for close\n";
    std::cout << "for help type <help>" << std::endl;

    // MAIN LOOP
    bool isover = false;
    while(!isover) {
        std::string request;
        std::cout << "Enter message: ";
        std::getline(std::cin,request);
        std::cout << std::endl;


        switch(check_cmd(request)) {
            case  0:
                     isover = true;
                     break;
            case  1:
                     std::cout << help << std::endl;
                     break;
            case  2:
                     A.emit_tokens(1);
                     break;
            case  3:
                     A.send_token_bynet(target_address, target_port);
                     break;
            case  4:
                     A.print_status(std::cout);
                     break;
            case  5:
                     A.save_user("user.dat");
                     break;
            case  6:
                     A.load_user("user.dat");
                     break;
            case  7:
                     set_interactive_target(target_address,target_port);
                     break;
            case -1:
                     std::cout << "bad command -- try again" << std::endl;
                     break;
            default:
                     std::cout << "bad check_cmd! -- check this" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(0));
    }
}

int main (int argc, char *argv[]) {
	try {
		ios_base::sync_with_stdio(false);
        int number_of_threads = 0;
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
        test_all(number_of_threads);

        //run_interactive_protocol();

        return 0;

	} catch (std::exception& e) {
        std::cerr << "MainException: " << e.what() << std::endl;
	}
}
