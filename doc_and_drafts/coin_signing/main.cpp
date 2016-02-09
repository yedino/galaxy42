#include <iostream>
#include "tests.hpp"


void run_interactive_protocol(std::string address, int port) {

    std::string my_name("bestcoin");

    c_netuser A(my_name, port);
    bool isover = false;

    std::cout << "Your target is <" << address << "> on tcp port <" << port << ">" << std::endl;

    std::string help = "help info:\n";
                help += "emit - emit new token\n";
                help += "send - send token to target\n";
                help += "status - print status info\n";
                help += "quit - for close\n";
    std::cout << "for help type <help>" << std::endl;

    while(!isover) {
        std::cout << "Enter message: ";
        std::string request;
        std::getline(std::cin,request);
        std::cout << std::endl;

        if(request == "quit") {
            isover = true;
            break;
        }
        else if(request == "emit") {
            A.emit_tokens(1);
        }
        else if(request == "send") {
            A.send_token_bynet(address,port);
        }
        else if(request == "status") {
            A.print_status(std::cout);
        }
        else if(request == "help") {
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

		if (argc <= 1) {
			cout << "please define number of theards to run test\n";
			return 0;
		}

        int number_of_threads = atoi(argv[1]);

		if (number_of_threads <= 0) {
			cout << "please define correct number of theards to run test\n";
			return 1;
        }

        test_all(number_of_threads);

        //run_interactive_protocol("127.0.0.1",30000);

        return 0;

	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}
