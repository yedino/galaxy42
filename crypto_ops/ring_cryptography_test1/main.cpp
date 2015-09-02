#include <iostream>
#include "c_simulation.hpp"
#include "cyclic_list.hpp"
#include <boost/circular_buffer.hpp>

using namespace std;

int main () {
	c_simulation sim;

	for (int i = 0; i < 5; ++i)
		sim.add_user();

	sim.initialize_simulation();
	sim.print_connections();
	sim.start_simluation();
	return 0;
}