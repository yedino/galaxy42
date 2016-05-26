#include <iostream>
using namespace std;

struct car { int speed; };

//struct parser;


// ----------

struct parser {
	template <typename T>
	T pop();
};

template<typename T>
constexpr
bool dependentFalse()
{
    return false;
}

template <typename T>
T deserialize(parser &p) {
	static_assert(dependentFalse<T>(), "Write own deserialize()");
	T ret;
	return ret;
}

template <typename T>
T parser::pop() {
	T ret( deserialize<T>( *this ) );
	return ret;
}


/*
template<>
car parser::pop<car>() {
	car ret;
	ret.speed=42;
	return ret;
}
*/
// ----------

// spec
template <>
car deserialize<car>(parser &p) {
	car ret;
	ret.speed=42;
	return ret;
}

struct train { float f; };

int main() {
	parser p;
	car c =	p.pop<car>();
//	train t =	p.pop<train>();

	cout << c.speed << endl;
}

