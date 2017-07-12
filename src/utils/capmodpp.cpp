
#include "platform.hpp" // just to detect OS type, is it ANTINET_linux



#include <iomanip>
#include "capmodpp.hpp"

#ifdef ANTINET_linux
#include <linux/types.h>
#include <limits>
#include <cap-ng.h>
#endif
#include <cstring>

#define debug_capmodpp 0
#if (debug_capmodpp)
	#pragma message "You are using special debug in this file. Do NOT DO THAT in production as it is insecure/UB"
#endif

namespace capmodpp {

/// @group typedefs to use in syntax auto x = type { .... }; while a C++ defect(?) disallows "multi-word" types.
/// @{
using t_const_char_ptr = const char *;
/// @}


capmodpp_error::capmodpp_error(const std::string & msg)
: m_msg(msg) { }

const char * capmodpp_error::what() const noexcept {
	return m_msg.c_str();
}

#ifdef ANTINET_linux
// ===========================================================================================================

// Wrapped functions
// CODE STYLE - I use variables fail and badval; Fail is when underlying function signaled error, or there was some error seen
// badval is when the returned value is not valid
// I use both even in cases where there only one of them is needed, so entire code-blocks is identical in all of them
// Could had used macro like assert_correct_result( (ret==CAPNG_FAIL) , ((ret==0)||(ret==1)) );
// but not using it so code is more readable for security review.

int secure_capng_have_capability(capng_type_t which, unsigned int capability) {
	auto ret = int { capng_have_capability(which,capability) };
	bool fail = (ret == CAPNG_FAIL);
	bool badval = ! (  (ret==0) || (ret==1)  );
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for which="<<which<<" capability="<<capability<<".";
		throw capmodpp_error(oss.str());
	}
	return ret;
}

capng_results_t secure_capng_have_capabilities(capng_select_t set) {
	if (!(  (set==CAPNG_SELECT_CAPS)||(set==CAPNG_SELECT_BOUNDS)||(set==CAPNG_SELECT_BOTH)  )) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__	<< ": set="<<set<<"." ;
		throw capmodpp_error(oss.str());
	}
	auto ret = capng_results_t { capng_have_capabilities(set) }; // *** main call
	bool fail = (ret == CAPNG_FAIL);
	bool badval = ! (  (ret==CAPNG_NONE) || (ret==CAPNG_PARTIAL) || (ret==CAPNG_FULL)  );
	static_assert(CAPNG_PARTIAL > 0 , "this value was promised to be greater-then 0.");
	static_assert(CAPNG_FULL > 0 , "this value was promised to be greater-then 0.");
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for set="<<set<<".";
		throw capmodpp_error(oss.str());
	}
	if ( (ret != CAPNG_FAIL) && (!( ret>=0 )) ) {
		std::ostringstream oss; oss<<"Error: " << " values other then CAPNG_FAIL were supposed to be >=0, but are not:"
			<< " (ret="<<ret<<") in " << __func__
			<< " for set="<<set<<".";
		throw capmodpp_error(oss.str());
	}
	return ret;
}

// Test whether a<=b, in a safe way that doesn't cause UB
bool safe_less_eq_than(int a, unsigned int b) {
	if (a<0) return true; // because b can't be below 0
	// now both a,b are >=0
	unsigned int a_unsign = static_cast<unsigned int>( a ); // it's unsigned so we can cast it like that
	return a_unsign <= b;
}

const char * secure_capng_capability_to_name(unsigned int capability) {
	static_assert(std::numeric_limits<decltype(capability)>::is_signed==false, "This must be unsigned (as we do not test for >=0)");
	bool inputok = (capability<=get_last_cap_nr());
	if (!inputok) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__
			<< " for capability="<<capability<<".";  // WARNING: output only values that are valid enough
		throw capmodpp_error(oss.str());
	}
	auto ret = t_const_char_ptr { capng_capability_to_name(capability) };
	bool fail = (ret==nullptr);
	bool badval = false;
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for capability="<<capability<<".";
		throw capmodpp_error(oss.str());
	}
	return ret;
}

int secure_capng_name_to_capability(const char *name) {
	if (name == nullptr) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__	<< " (name was null).";
		throw capmodpp_error(oss.str());
	}
	auto len = size_t { std::strlen(name) };
	if ( (len<=0) || (len > max_expected_cap_name_length)) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__
			<< "(name had invalid length len="<<len<<").";
		throw capmodpp_error(oss.str());
	}
	auto ret = int { capng_name_to_capability(name) };
	bool fail = (ret==-1);
	bool badval = ! ( (ret>=0) && safe_less_eq_than(ret , get_last_cap_nr()) );
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for name="<<name<<".";  // WARNING: output only values that are valid enough
		throw capmodpp_error(oss.str());
	}
	return ret;
}

void secure_capng_get_caps_process() {
	auto ret = int { capng_get_caps_process() }; /// does syscall to actually read state
	bool fail = (ret!=0);
	bool badval = false;
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< "."
			;
		throw capmodpp_error(oss.str());
	}
}

void secure_capng_update(capng_act_t action, capng_type_t type,unsigned int capability) {
	if (!(  (action==CAPNG_DROP)||(action==CAPNG_ADD)   )) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__	<< ": action="<<action<<"." ;
		throw capmodpp_error(oss.str());
	}
	if (!(  (type==CAPNG_EFFECTIVE)||(type==CAPNG_PERMITTED)||(type==CAPNG_INHERITABLE)||(type==CAPNG_BOUNDING_SET)  )) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__	<< ": type="<<type<<"." ;
		throw capmodpp_error(oss.str());
	}

	if (!(  (capability<=get_last_cap_nr()) )) {
		static_assert(std::numeric_limits<decltype(capability)>::is_signed==false, "This must be unsigned (as we do not test for >=0)");
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__ << ": capability="<<capability<<".";
		throw capmodpp_error(oss.str());
	}

	auto ret = int { capng_update(action,type,capability) };
	bool fail = (ret!=0);
	bool badval = false;
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for action="<<action<<",type="<<type<<"capability="<<capability;  // WARNING: output only values that are valid enough
		throw capmodpp_error(oss.str());
	}
}

void secure_capng_apply(capng_select_t set) {
	if (!(  (set==CAPNG_SELECT_CAPS)||(set==CAPNG_SELECT_BOUNDS)||(set==CAPNG_SELECT_BOTH)   )) {
		std::ostringstream oss; oss<<"Error: " << "invalid input" << " in " << __func__	<< ": set="<<set<<"." ;
		throw capmodpp_error(oss.str());
	}
	auto ret = int { capng_apply(set) };
	bool fail = (ret != 0);
	bool badval = false;
	if (fail||badval) {
		std::ostringstream oss; oss<<"Error: " << (fail ? "FAILED":"") << " " << (badval ? "BAD-VALUE":"")
			<< " (ret="<<ret<<") in " << __func__
			<< " for set="<<set<<".";
		throw capmodpp_error(oss.str());
	}
}


// ===========================================================================================================

const t_eff_value v_eff_enable{cap_permchange::enable};
const t_eff_value v_eff_disable{cap_permchange::disable};
const t_eff_value v_eff_unchanged{cap_permchange::unchanged};

const t_permit_value v_permit_enable{cap_permchange::enable};
const t_permit_value v_permit_disable{cap_permchange::disable};
const t_permit_value v_permit_unchanged{cap_permchange::unchanged};

const t_inherit_value v_inherit_enable{cap_permchange::enable};
const t_inherit_value v_inherit_disable{cap_permchange::disable};
const t_inherit_value v_inherit_unchanged{cap_permchange::unchanged};

bool operator!(cap_perm value) noexcept {
	return value != cap_perm::yes;
}
bool is_true(cap_perm value) noexcept {
	return value == cap_perm::yes;
}


cap_nr get_last_cap_nr() noexcept {
	return CAP_LAST_CAP;
}

cap_nr get_cap_size() noexcept {
	auto constexpr last = CAP_LAST_CAP;
	// can we add +1 without overflow, here:
	static_assert( std::numeric_limits<cap_nr>::max() - last >= 1 , "CAP_LAST_CAP would overflow" );
	return CAP_LAST_CAP+1;
}

std::string cap_nr_to_name(cap_nr nr) {
	if (nr > CAP_LAST_CAP) {
		std::ostringstream oss; oss << "invalid_cap_" << nr;
		throw std::runtime_error(oss.str());
	}
	return capmodpp::secure_capng_capability_to_name(nr);
}

cap_nr cap_name_to_nr(const std::string & name) {
	auto nr = capmodpp::secure_capng_name_to_capability(name.c_str());
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

bool cap_state::is_any() const {
	if (this->eff == cap_perm::yes) return true;
	if (this->permit == cap_perm::yes) return true;
	if (this->inherit == cap_perm::yes) return true;
	if (this->bounding == cap_perm::yes) return true;
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
	ostr <<  ( (obj == cap_perm::yes) ? "YES" : "no." ) ;
	return ostr;
}

std::ostream & operator<<(std::ostream & ostr, const cap_state & obj) {
	ostr << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return ostr;
}

void cap_state_map::print(std::ostream & ostr, int level) const {
	if (level == 0) { // one-liner version
		ostr << "CAP state: ";
		auto count_all = this->state.size();
		size_t count_usable=0, count_any=0;
		std::ostringstream detail;
		bool first=true;
		for(const auto & item : this->state) {
			bool show=false;
			if (item.second.is_usable()) { ++count_usable; show=true; }
			if (item.second.is_any()) { ++count_any; show=true; }
			if (show) {
				if (!first) detail << ", ";
				first=false;
				detail << secure_capng_capability_to_name(item.first);
				detail << "+";
				if (is_true(item.second.eff)) detail<<"e";
				if (is_true(item.second.permit)) detail<<"p";
				if (is_true(item.second.inherit)) detail<<"i";
				if (is_true(item.second.bounding)) detail<<"b";
			}
		}
		ostr << " count: any=" << count_any << ", usable="<<count_usable<<" / "<<count_all << " [" << detail.str() << "]";
	}
	else { // verbose version
		ostr << "CAP state with " << this->state.size() << " CAPs defined:\n";
		size_t skipped=0;
		for(const auto & item : this->state) {
			bool interesting = item.second.is_usable();
			bool bounding = is_true(item.second.bounding);
			if ( (level>=20) || (interesting) ) {
				ostr << std::setw(2) << item.first << ": " << item.second << " ";
				if (interesting) { ostr << "******"; }
					else if (bounding) { ostr << " (b)  "; }
					else ostr << "      ";
				ostr << " " << cap_nr_to_name(item.first) << "\n";
			} else ++skipped;
		}
		if (skipped) ostr << "Skipped " << skipped << " non-interesting items" << "\n";
	}
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
	secure_capng_get_caps_process(); // read current state from syscall (igoring cached libng state)
	for (cap_nr nr=0; nr<=CAP_LAST_CAP; ++nr) { // including the CAP_LAST_CAP (as it is alias for some valid CAP)
		cap_state state;
		state.eff      = (secure_capng_have_capability(CAPNG_EFFECTIVE    , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.permit   = (secure_capng_have_capability(CAPNG_PERMITTED    , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.inherit  = (secure_capng_have_capability(CAPNG_INHERITABLE  , nr) > 0) ? cap_perm::yes : cap_perm::no;
		state.bounding = (secure_capng_have_capability(CAPNG_BOUNDING_SET , nr) > 0) ? cap_perm::yes : cap_perm::no;
		statemap.state.emplace(nr,state);
	}
	return statemap;
}

std::ostream & operator<<(std::ostream & ostr, const cap_statechange & obj) {
	ostr << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return ostr;
}

void cap_statechange_map::set(const std::string & capname , cap_statechange change) {
	this->state.emplace( cap_name_to_nr(capname) , change);
}

void cap_statechange_map::print(std::ostream & ostr, int level) const {
	size_t skipped{0};
	ostr << "CAP state CHANGE with " << this->state.size() << " CAP CHANGES defined:\n";
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

void cap_statechange_full::security_apply_now() const {
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
		secure_capng_get_caps_process(); // cap-ng state = read current state from syscall (igoring cached libng state)

		// apply differences (old to new) to the cap-ng state:
		for(const auto & item : state_new.state) {
			const auto nr = item.first;
			const auto cap_new = item.second;
			const auto cap_old = state_old.state.at(nr);

			if (dbg) std::cerr<<"applying nr = " << nr << std::endl;

			if (cap_new.bounding != cap_old.bounding) {
				if (dbg) std::cerr<<"applying BOUND nr = " << nr << std::endl;
				secure_capng_update( (cap_new.bounding == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_BOUNDING_SET,    nr);
			}
			if (cap_new.permit != cap_old.permit) {
				if (dbg) std::cerr<<"applying PERMIT nr = " << nr << std::endl;
				secure_capng_update( (cap_new.permit   == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_PERMITTED   ,    nr);
			}
			if (cap_new.eff != cap_old.eff) {
				if (dbg) std::cerr<<"applying EFF nr = " << nr << std::endl;
				secure_capng_update( (cap_new.eff      == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_EFFECTIVE   ,    nr);
			}
			if (cap_new.inherit != cap_old.inherit) {
				if (dbg) std::cerr<<"applying INHERIT nr = " << nr << std::endl;
				secure_capng_update( (cap_new.inherit  == cap_perm::yes) ? CAPNG_ADD : CAPNG_DROP, CAPNG_INHERITABLE ,    nr);
			}
		}
		secure_capng_apply(CAPNG_SELECT_BOTH); // <--- actually apply the cap-ng state to system
	}

}

#endif

}//namespace capmodpp

#undef debug_capmodpp



