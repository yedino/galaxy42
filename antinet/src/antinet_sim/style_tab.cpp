#include <algorithm>
#include <set>

namespace test {

int foo(int a, int b) {
    std::set<int> t;

    if (a>b) { // 1 tab
        int x = a+b;
        int y = a*b // 2 tab
              + a // 2 tab, plus 6 space to align
              + b
              + 42
              + std::count(
                t.begin(), // 2 tab, 6 space, again 1 tab
                t.end(),
                [](int p) -> bool {
                    if (a<0) { // 2 tab, 6 space, again 2 tab
                      return a%2; // 2 tab, 6 space, again 3 tab
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
