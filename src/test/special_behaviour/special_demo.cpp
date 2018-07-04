
/**
 * @see README.txt here , do not cover this with unit-tests
 */

#include "special_demo.hpp"

#include "libs0.hpp"

namespace n_special_behaviour {

void example_warn_1() {
	pfp_warn("Example warning printed out");
}

void example_memcheck_1() { // to test valgrind detection of errors
	pfp_mark("Valgrind memcheck test.");
	vector<int> vec(100);
	int* ptr = & vec.at(0);
	ptr += 200;
	volatile int read = *ptr;
	volatile int result = (read==0) ? 0 : 1; // to silence "unused var" and to cause e.g. cause "jump depends on uninitialized"
	pfp_mark("Valgrdin memcheck test done (program not aborted), result"<<result);
}

void example_memcheck_2() { // to test valgrind detection of errors
	pfp_mark("Valgrind memcheck test.");
	vector<int> vec(100);
	vec[2000] = 42;
	volatile auto * ptr = & vec[3000];
	*ptr = 42;
	pfp_mark("Valgrdin memcheck test done (program not aborted), result");
}

void example_ubsan_1() { // to test UBSAN / ub sanitize
	pfp_mark("UBSAN test.");
	// TODO make it work for "signed char" as well
	signed int i;
	i = std::numeric_limits<decltype(i)>::max();
	i += static_cast<decltype(i)>(5); // overflow of signed
	pfp_mark("UBSAN test done (program not aborted)");
}

void example_tsan_1() { // to test STAN / thread sanitizer
	pfp_mark("TSAN test.");
	int data=10;
	// run concurent update of data:
	auto thread1 = std::thread( [&](){ data=20; } );
	auto thread2 = std::thread( [&](){ data=30; } );
	thread1.join();
	thread2.join();
	pfp_mark("TSAN test done (program not aborted)");
}

} // n_special_behaviour

