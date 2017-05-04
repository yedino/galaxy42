
#pragma once

namespace stdplus {

/*
template <typename Base, typename Derived>
class cloneable : public Base
{
	public:
		using Base::Base;

		or rather return unique_ptr<Base> ? it's usually the same class used as Base template,
		so we do not need to change return type (which is possible only for dervided classes).

		Then we must use the same Base, as smart pointers are not covaraitn types:
		http://stackoverflow.com/questions/196733/how-can-i-use-covariant-return-types-with-smart-pointers
		http://stackoverflow.com/questions/6924754/return-type-covariance-with-smart-pointers

		virtual Base *clone() const {
			return new Derived(static_cast<Derived const &>(*this));
		}
};
*/

}

