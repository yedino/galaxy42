
#include <platform.hpp>
#include "capmodpp.hpp"


#ifdef ANTINET_linux
	#include <linux/types.h>

	// uses libcap-ng
	#include <cap-ng.h>
#else
	#error "not supported"
#endif


namespace capmodpp {

cap_state::cap_state() : eff(cap_perm::no), permit(cap_perm::no), inherit(cap_perm::no), bounding(cap_perm::no)
{ }

std::ostream & operator<<(std::ostream & oss, const cap_perm & obj) {
	oss <<  ( (obj == cap_perm::yes) ? "YES" : "no" ) ;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_state & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_state_map & obj) {
	oss << "CAP state with " << obj.state.size() << " CAPs defined:\n";
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

// ===========================================================================================================

cap_statechange::cap_statechange()
: eff(cap_permchange::unchanged), permit(cap_permchange::unchanged),
	inherit(cap_permchange::unchanged), bounding(cap_permchange::unchanged)
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

std::ostream & operator<<(std::ostream & oss, const cap_permchange & obj) {
	switch (obj) {
		case cap_permchange::unchanged: oss<<"    same   "; break;
		case cap_permchange::enable:    oss<<"  +ENABLE+ "; break;
		case cap_permchange::disable:   oss<<" -disable- "; break;
	}
	return oss;
}

cap_state_map read_process_caps() {
	cap_state_map statemap;
	for (cap_nr nr=0; nr<CAP_LAST_CAP; ++nr) 
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange_map & obj) {
	oss << "CAP changes with " << obj.state.size() << " CAPs defined:\n";
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange_full & obj) {
	oss << obj.given << "\n" << "all_others: " << obj.all_others ;
	return oss;
}

}//namespace capmodpp
