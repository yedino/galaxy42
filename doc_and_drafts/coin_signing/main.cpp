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
// if address is incorect set addres to localhost [::1]
bool checkIPv6_address(std::string & address) {
  try {
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(address, ec);
    if (!addr.is_v6()) {
        std::string msg = addr.to_string();
        msg += " [" + address + "] : is not valid IPv6 address";
        throw std::invalid_argument(msg);
    } else if (addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " [" + address + "] : is valid IPv4 address but we using IPv6 addresses";
        throw std::invalid_argument(msg);
    } else if(ec) {
        throw std::invalid_argument(ec.message());
    }
  } catch (std::invalid_argument &err) {
        std::cout << err.what() << std::endl;
        address = "::1";
        return false;
  }
    return true;	// address is OK
}
// Check if port is correct for us. If not returning default port.
unsigned short get_port() {
  try {
    std::string input;
    if(!(std::cin >> input)) {
        std::string msg = "Invalid port number [" + input + "]";
        throw std::invalid_argument(msg);
    }
    int port = std::stoi(input);
    if(port > 1025 && port < 32000) {
        std::cin.ignore();
        return port;	// port is OK
    } else {
        std::string msg = "Invalid port number [" + std::to_string(port) + "]";
        throw std::invalid_argument(msg);
    }
  } catch (std::invalid_argument &err) {
        std::cout << err.what() << std::endl;
        std::cout << "Set to default port: 30000" << std::endl;
        std::cin.clear();
        std::cin.ignore();
        return 30000;
  }
}

bool set_interactive_target(std::string &address, int &port) {
    std::cout << "Enter target addres: ";
    std::getline(std::cin,address);

    std::cout << "Enter target port: ";
    port = get_port();

    return checkIPv6_address(address);
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
    std::cout << "Set your server port for this session: ";
    int my_port = get_port();

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
        if(!set_interactive_target(target_address,target_port)) {
            std::cout << "Fail to set correct target, try again by typing target command" << std::endl;
        }
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
        std::cin.clear();
        std::getline(std::cin,request);
        std::cout << request << std::endl;


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
                     if(!set_interactive_target(target_address,target_port)) {
                         std::cout << "Fail to set correct target, try again by typing target command" << std::endl;
                     }
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

// TODO move to utils
template <typename TCont, typename TValue> 
bool any_of_is(const TCont & cont, const TValue & target_value) {
	return std::any_of(cont.begin(), cont.end(), 
		[target_value](const TValue & this_element) -> bool { return this_element == target_value; } 
	);
}

using namespace std;


int main (int argc, char *argv[]) {
	vector<string> argtab;
	for (auto i=1; i<argc; ++i) argtab.push_back(argv[i]);

	typedef enum { e_mainfunction_interactive , e_mainfunction_tests , e_mainfunction_test_serialize } t_mainfunctions;
	t_mainfunctions mainfunction = e_mainfunction_interactive;

	if (any_of_is(argtab, string("--test2"))) mainfunction = e_mainfunction_test_serialize;
    if (any_of_is(argtab, string("--test"))) mainfunction = e_mainfunction_tests;


	try {
		ios_base::sync_with_stdio(false);

        if (mainfunction == e_mainfunction_tests) {        	

            int number_of_threads = 0;
            bool correct_threads_num = true;

            if (argc <= 2) {
                std::cout << "you could define number of theards in argv[1] to run test with" << std::endl;
                correct_threads_num = false;
            } else {
                    auto number_of_threads_str = argtab.at(2);
                number_of_threads = atoi( number_of_threads_str.c_str() );
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

        }

				if (mainfunction == e_mainfunction_interactive) {
        	run_interactive_protocol();
        }

				if (mainfunction == e_mainfunction_test_serialize) {
					cout << "Test serialize" << endl;

				}

        return 0;

	} catch (std::exception& e) {
        std::cerr << "MainException: " << e.what() << std::endl;
	}
}
