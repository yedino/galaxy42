#include <boost/asio.hpp>
#include <iostream>

#include "libs0.hpp"

#ifndef ANTINET_PART_OF_YEDINO
	#error "Need to define ANTINET_PART_OF_YEDINO (e.g. from project.hpp)"
#endif


// warning: including .cpp code here
// (untill CMake update - @rob) TODO

#include "../src-tools/netmodel/netmodel.cpp"


void asiotest()
{
	int ret = n_netmodel::netmodel_main(0,nullptr); // TODO args
	UNUSED(ret);
}

