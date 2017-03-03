#include "cable/simulation/cable_simul_addr.hpp"


c_cable_simul_addr::c_cable_simul_addr(int uuid) : c_cable_base_addr(uuid) { }

void c_cable_simul_addr::print(ostream & ostr) const {
	auto uuid = boost::any_cast<int>(get_addrdata());
	ostr << "[transp-simul-" << uuid << "]" ;
}

bool c_cable_simul_addr::is_same(const c_cable_base_addr &other) const {
	try {
		return any_cast<t_addr>(other.get_addrdata())  ==  any_cast<t_addr>(get_addrdata());
	} catch(...) {
		return 0; // the other address has even other type then me, so it's different
	}
}

int c_cable_simul_addr::compare(const c_cable_base_addr &other) const {
	try {
		const t_addr & my_addr    = any_cast<t_addr>(this->get_addrdata());
		const t_addr & other_addr = any_cast<t_addr>(other.get_addrdata());
		if (my_addr < other_addr) return -1;
		if (other_addr < my_addr) return +1;
		_check(my_addr == other_addr);
		return 0; // same
	} catch(...) {
		return this->cable_type_name().compare( other.cable_type_name() );
	}
}

