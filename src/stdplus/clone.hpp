
#pragma once

namespace stdplus {

template <typename Base, typename Derived>
class cloneable : public Base
{
	public:
		using Base::Base;

		virtual Base *clone() const {
			return new Derived(static_cast<Derived const &>(*this));
		}
};

}

