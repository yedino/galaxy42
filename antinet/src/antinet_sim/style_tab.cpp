#include <algorithm>
#include <list>
#include <iostream>

namespace test
{

int foo(int a, int b)
{
	std::list<int> t;
	if (a>b) { // 1 tab

		std::cout << "This is "
			<< "a test "
			<< "variable is: "
			<< a
		;

		// rare problems:

			std::cout << "Hello " // 2 tab
			          << "world! " // 2 tab, and hand allign with spaces
			          << a
			          ;

			int x = a+b;
			int y = a*b // 2 tab
			        + a // 2 tab, plus 6 space to align
			        + b
			        + 42
			        + std::count_if(
			          	t.begin(), // 2 tab, 6 space, again 1 tab
			          	t.end(),
			          	[](int p) {
			          		int d=p*2;
			          		if (d>10) {
			          			return p%2;
			          		}
			          		return p%4;
			          	} // lambda
			        ) // count
			        +1000;
			int z=x+y;
			if (z) return z;
		} // a>b

	return a+b;
}

}
