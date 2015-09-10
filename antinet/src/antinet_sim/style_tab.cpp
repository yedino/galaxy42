#include <algorithm>
#include <set>

namespace test { 

int foo(int a, int b) {
	std::set<int> t;

	if (a>b) { // 1 tab
		int x = a+b;
		int y = a*b // 2 tab a // 2 tab + 7 space
		      + a
		      + b
		      + 42
		      + std::count(
		      	t.begin(), 
		      	t.end(), 
		      	[](int p) -> bool {
		      		if (a<0) {
		      			return a%2;
		      		}
		      		return a%4;
		      	} // lambda
		      ) // count
		      +1000;
		int z=x+y;
	} // a>b

	return a,b;
}

}
