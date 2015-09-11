#include <algorithm>
#include <list>
#include <iostream>

namespace test {

int bar (int x) {
	return 42;
}

int foo (int a, int b) {
	int a = bar();
	
	std::list<int> t;
	
	if (a) {
		// ...
	}
	
	if (a.has_erro()) return 4;
	if (a.has_warning()) return 5;
	if (a.has_warning2()) return 5;
	if (a.has_warning3(plus1)) return 5;
	
	if ( (a==b)   ||    (   (a>b) && (b<a) && ((a|b) > 5)   )) { } ;
	if ( (a==b)   ||    (   (a>b) && (b<a) && ((a|b) > 5)   )) { } ;
	if ( (a==b)   ||    (   (a>b) && (b<a) && ((a|b) > 5)   )) { } ;
	if ( (a==b)   ||    (   (a>b) && (b<a) && ((a|b) > 5)   )) { } ;
	
	if (a>b) { // 1 tab

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
