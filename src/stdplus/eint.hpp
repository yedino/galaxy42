
#include <sstream>
#include <stdexcept>
#include <typeinfo>

namespace stdplus {

/**
 * @namespace eint - exception int - function for safe integer operationsm that will throw exception if runtime arguments used
 * would result in mathemathically incorrect result, remember to assign the returned values in safe initializer that can not
 * narrow the argument, e.g.  unsigned int x{ eint_minus(a,b); } for compile-time check against incorrect type when
 * type of e.g. "a" is narrower then type of "x".
 * @warning remember to use aggregate initialization to avoid the narrowing as above.
 * @see http://eel.is/c++draft/dcl.init.aggr
 * @see http://en.cppreference.com/w/cpp/language/aggregate_initialization
 * @owner rfree
 */
namespace eint {

/// Calculate (a-b) in safe way (see documentation and WARNINGS about this namespace)
/// @TODO now assumes T is UNSIGNED type!
/// @TODO can variable "b" be silently converted to type T here, including narrowing? if yes then template on T1,T2 and
/// ...check if b can be properly expanded to T1 ?
template <typename T>
T eint_minus(T a, T b) {
	if (a<b) {
		std::ostringstream oss; oss<<"Math error when calculating (" << a << " - " << b << ") for T="<<typeid(T).name();
		throw std::range_error(oss.str());
	}
	T c = a - b;
	return c;
}

} // namespace eint

} // namespace stdplus
