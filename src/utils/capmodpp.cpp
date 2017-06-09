
#include <platform.hpp>
#include <iomanip>
#include "capmodpp.hpp"


#ifdef ANTINET_linux
	#include <linux/types.h>

	// uses libcap-ng
	#include <cap-ng.h>
#else
	#error "not supported"
#endif

#define debug_capmodpp 0

namespace capmodpp {

const t_eff_value v_eff_enable{cap_permchange::enable};
const t_eff_value v_eff_disable{cap_permchange::disable};
const t_eff_value v_eff_unchanged{cap_permchange::unchanged};

const t_permit_value v_permit_enable{cap_permchange::enable};
const t_permit_value v_permit_disable{cap_permchange::disable};
const t_permit_value v_permit_unchanged{cap_permchange::unchanged};

const t_inherit_value v_inherit_enable{cap_permchange::enable};
const t_inherit_value v_inherit_disable{cap_permchange::disable};
const t_inherit_value v_inherit_unchanged{cap_permchange::unchanged};

cap_nr get_last_cap_nr() {
	return CAP_LAST_CAP;
}

cap_nr get_cap_size() {
	return CAP_LAST_CAP+1;
}

std::string cap_nr_to_name(cap_nr nr) {
	if (nr > CAP_LAST_CAP) {
		std::ostringstream oss; oss << "invalid_cap_" << nr;
		throw std::runtime_error(oss.str());
	}
	return capng_capability_to_name(nr);
}

cap_nr cap_name_to_nr(const std::string & name) {
	auto nr = capng_name_to_capability(name.c_str());
	long int nr_long{ nr };
	long int last_long{ get_last_cap_nr() };
	if ((nr_long >= last_long ) || (nr<0)) {
		std::ostringstream oss; oss << "Can not convert cap name (" << name << ") to a valid cap number, got: " << nr;
		throw std::runtime_error(oss.str());
	}
	return nr;
}

cap_state::cap_state() : eff(cap_perm::no), permit(cap_perm::no), inherit(cap_perm::no), bounding(cap_perm::no)
{ }

bool cap_state::is_usable() const {
	if (this->eff == cap_perm::yes) return true;
	if (this->permit == cap_perm::yes) return true;
	if (this->inherit == cap_perm::yes) return true;
	return false;
}

void apply_change_perm(cap_perm & perm , const cap_permchange & change) {
	switch (change) {
		case cap_permchange::enable: perm = cap_perm::yes; break;
		case cap_permchange::disable: perm = cap_perm::no; break;
		case cap_permchange::unchanged: break;
	}
}

void cap_state::apply_change(const cap_statechange & change) {
	apply_change_perm( this->eff, change.eff );
	apply_change_perm( this->permit, change.permit );
	apply_change_perm( this->inherit, change.inherit );
	apply_change_perm( this->bounding, change.bounding );
	if (debug_capmodpp) { std::cerr << "After applying changes " << change << " I am : " << (*this) << std::endl; }
}

std::ostream & operator<<(std::ostream & ostr, const cap_perm & obj) {
	ostr <<  ( (obj == cap_perm::yes) ? "YES" : "no " ) ;
	return ostr;
}

std::ostream & operator<<(std::ostream & ostr, const cap_state & obj) {
	ostr << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return ostr;
}

void cap_state_map::print(std::ostream & ostr, int level) const {
	ostr << "CAP state with " << this->state.size() << " CAPs defined:\n";
	size_t skipped=0;
	for(const auto & item : this->state) {
		bool interesting = item.second.is_usable();
		if ( (level>=20) || (interesting) ) {
			ostr << std::setw(2) << item.first << ": " << item.second
				<< " " << (interesting ? "******" : "      ")
				<< " " << cap_nr_to_name(item.first)
				<< "\n";
		} else ++skipped;
	}
	if (skipped) ostr << "Skipped " << skipped << " non-interesting items" << "\n";
}

std::ostream & operator<<(std::ostream & ostr, const cap_state_map & obj) {
	obj.print(ostr,10);
	return ostr;
}

// ===========================================================================================================

cap_statechange::cap_statechange()
:
eff(cap_permchange::unchanged),
permit(cap_permchange::unchanged),
inherit(cap_permchange::unchanged),
bounding(cap_permchange::unchanged)
{ }

cap_statechange::cap_statechange(t_eff_value given_eff, t_permit_value given_permit, t_inherit_value given_inherit)
:
eff(given_eff.value),
permit(given_permit.value),
inherit(given_inherit.value),
bounding(cap_permchange::unchanged)
{ }

cap_statechange & cap_statechange::set(cap_area_type area, cap_permchange value) {
	switch(area) {
		case cap_area_type::eff:      this->eff=value; break;
		case cap_area_type::permit:   this->permit=value; break;
		case cap_area_type::inherit:  this->inherit=value; break;
		case cap_area_type::bounding: this->bounding=value; break;
	}
	return *this;
}

bool cap_statechange::is_change() const {
	if (this->eff != cap_permchange::unchanged) return true;
	if (this->permit != cap_permchange::unchanged) return true;
	if (this->inherit == cap_permchange::unchanged) return true;
	if (this->bounding == cap_permchange::unchanged) return true;
	return false;
}

std::ostream & operator<<(std::ostream & ostr, const cap_permchange & obj) {
	switch (obj) {
		case cap_permchange::unchanged: ostr<<"(same)"; break;
		case cap_permchange::enable:    ostr<<"ENABLE"; break;
		case cap_permchange::disable:   ostr<<"remove"; break;
	}
	return ostr;
}

cap_state_map read_process_caps() {
	cap_state_map statemap;
	capng_get_caps_process(); // explicit re-read (though not 100% sure if capng isn't caching it there)
	for (cap_nr nr=0; nr<CAP_LAST_CAP; ++nr) {
		cap_state state;
		state.eff      = (capng_have_capability(CAPNG_EFFECTIVE    , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.permit   = (capng_have_capability(CAPNG_PERMITTED    , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.inherit  = (capng_have_capability(CAPNG_INHERITABLE  , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.bounding = (capng_have_capability(CAPNG_BOUNDING_SET , nr) > 0) ? cap_perm::yes : cap_perm::no;
		statemap.state[nr] = state;
	}
	return statemap;
}

std::ostream & operator<<(std::ostream & ostr, const cap_statechange & obj) {
	ostr << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return ostr;
}

void cap_statechange_map::set(const std::string & capname , cap_statechange change) {
	this->state[ cap_name_to_nr(capname) ] = change;
}

void cap_statechange_map::print(std::ostream & ostr, int level) const {
	size_t skipped=0;
	for(const auto & item : this->state) {
		bool interesting = item.second.is_change();
		if ( (level>=20) || (interesting) ) {
			ostr << std::setw(2) << item.first << ": " << item.second
				<< " " << (interesting ? " ******" : "      ")
				<< " " << cap_nr_to_name(item.first)
				<< "\n";
		} else ++skipped;
	}
	if (skipped) ostr << "Skipped " << skipped << " non-interesting items" << "\n";

}

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_map & obj) {
	obj.print(ostr,10);
	return ostr;
}

void cap_statechange_full::print(std::ostream & ostr, int level) const {
	this->given.print(ostr, level);
	ostr << "all_others: " << this->all_others ;
}

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_full & obj) {
	obj.print(ostr);
	return ostr;
}

void cap_statechange_full::set_given_cap(const std::string & capname , cap_statechange change) {
	given.set(capname,change);
}
void cap_statechange_full::set_all_others(cap_statechange change) {
	all_others = change;
}

void cap_statechange_full::security_apply_now() {
	const bool dbg = debug_capmodpp;

	// (1) read current privileges set - state_old
	// (2) make copy, and (2b) modify that set - state_new
	// (3) for each differnce between state_old and state_new, apply (cap-ng) the change
	// (4) really apply the cap-ng state to the system

	capmodpp::cap_state_map state_old = capmodpp::read_process_caps(); // (1)
	if (dbg) std::cerr << "=== OLD === Read state_old as: " << state_old << std::endl;
	capmodpp::cap_state_map state_new = state_old; // (2)

	{ // apply our capmodpp state changes to variable state_new
		for(auto & item : state_new.state) {
			const auto nr = item.first;
			auto & cap = item.second;
			if (this->given.state.count(nr)) { // if we have a change for that CAP nr
				if (dbg) std::cerr << "apply changes for nr=" << nr << " (custom rule for this one...) " << std::endl;
				cap.apply_change( this->given.state.at(nr) );
			}
			else {
				if (dbg) std::cerr << "apply changes for nr=" << " (as all_others)" << std::endl;
				cap.apply_change( this->all_others );
			}
		}
	}

	{
		capng_get_caps_process(); // cap-ng state = read current state

		// apply differences (old to new) to the cap-ng state:
		for(const auto & item : state_new.state) {
			const auto nr = item.first;
			const auto cap_new = item.second;
			const auto cap_old = state_old.state.at(nr);

			if (dbg) std::cerr<<"applying nr = " << nr << std::endl;

			if (cap_new.bounding != cap_old.bounding) {
				if (dbg) std::cerr<<"applying BOUND nr = " << nr << std::endl;
				auto ret = capng_update( (cap_new.bounding == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_BOUNDING_SET,    nr);
				if (ret!=0) throw std::runtime_error("Can not apply CAP flag (bounding)");
			}
			if (cap_new.permit != cap_old.permit) {
				if (dbg) std::cerr<<"applying PERMIT nr = " << nr << std::endl;
				auto ret = capng_update( (cap_new.permit   == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_PERMITTED   ,    nr);
				if (ret!=0) throw std::runtime_error("Can not apply CAP flag");
			}
			if (cap_new.eff != cap_old.eff) {
				if (dbg) std::cerr<<"applying EFF nr = " << nr << std::endl;
				auto ret = capng_update( (cap_new.eff      == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_EFFECTIVE   ,    nr);
				if (ret!=0) throw std::runtime_error("Can not apply CAP flag");
			}
			if (cap_new.inherit != cap_old.inherit) {
				if (dbg) std::cerr<<"applying INHERIT nr = " << nr << std::endl;
				auto ret = capng_update( (cap_new.inherit  == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_INHERITABLE ,    nr);
				if (ret!=0) throw std::runtime_error("Can not apply CAP flag");
			}
		}
		capng_apply(CAPNG_SELECT_BOTH); // <--- actually apply the cap-ng state to system
	}

}


}//namespace capmodpp


