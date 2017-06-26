

#include "my_tests_lib.hpp"

#include "../tnetdbg.hpp"
#include "../utils/privileges.hpp"

#include <exception>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <type_traits>

using namespace std;

#ifndef ANTINET_linux

// nothing to test on non-linux
TEST( privileges , nothing_to_test_on_this_platform ) {
}

#else

#include <linux/types.h>

#include "../utils/capmodpp.hpp"

// uses libcap-ng - to compare some things like CAP_LAST_CAP
#include <cap-ng.h>

namespace detail {

using t_rawcaps = map<string,long int>; // raw caps as read from e.g. /proc/self/status, string nme => value (as integer)

t_rawcaps read_slash_proc_rights() {
	t_rawcaps caps;
	const std::string thefilename("/proc/self/status");
	std::ifstream thefile(thefilename);
	while (thefile.good()) {
		string line="";
		std::getline(thefile,line);
		_dbg3("line: "<<line);
		if (line.find("Cap")==0) {
			_dbg2("has cap");
			string sep=":\t";
			size_t space_pos=line.find(sep);
			if (space_pos != string::npos) {
				string name(line.substr(0,space_pos));
				bool name_ok=false;
				if (name=="CapEff") name_ok=true;
				if (name=="CapPrm") name_ok=true;
				if (name=="CapInh") name_ok=true;
				if (name=="CapBnd") name_ok=true;
				if (name_ok) {
					string data = line.substr(space_pos + sep.size());
					t_rawcaps::mapped_type data_int;
					std::istringstream iss(data);
					iss >> std::hex >> data_int;
					bool ok = ( (!iss.fail()) && (iss.eof()) );
					if (!ok) _throw_error_runtime("Can not parse Caps (from thefile "s + thefilename + ") - the line is: ["s + line + "]"s );
					caps.emplace(name,data_int); // insert new element
				} // name
			} // sep
		} // Cap
	} // loop all lines of status
	_dbg1("Read caps from "<<thefilename<<" - size: " << caps.size());
	return caps;
}

} // namespace detail


TEST( privileges , name_cap_numbers ) {
	EXPECT_EQ( static_cast<long int>(capmodpp::get_last_cap_nr()) , static_cast<long int>(CAP_LAST_CAP) );
	EXPECT_EQ( static_cast<long int>(capmodpp::get_cap_size()) , static_cast<long int>(CAP_LAST_CAP + 1) );
	EXPECT_GT( static_cast<long int>(capmodpp::get_cap_size()) , 5); // there are always more then this CAPs
}

TEST( privileges , name_and_capnr_conversions ) {
	// example cap number that does not exist
	// need to update this example if kernel would include such cap in future
	EXPECT_THROW( { auto x = capmodpp::cap_nr_to_name(100); volatile auto dump = x; } , std::runtime_error );

	EXPECT_THROW( { auto x = capmodpp::cap_nr_to_name(CAP_LAST_CAP + 1 ); volatile auto dump = x; } , std::runtime_error );
	EXPECT_THROW( { auto x = capmodpp::cap_nr_to_name( capmodpp::get_last_cap_nr() + 1 ); volatile auto dump = x; } , std::runtime_error );
	EXPECT_NO_THROW( { auto x = capmodpp::cap_nr_to_name(CAP_LAST_CAP); volatile auto dump = x; } );
	EXPECT_NO_THROW( { auto x = capmodpp::cap_nr_to_name( capmodpp::get_last_cap_nr()); volatile auto dump = x; } );

	// example cap name that does not exist
	// need to update this example if kernel would include such cap in future :)
	EXPECT_THROW( capmodpp::cap_name_to_nr("MINE_BITCOINS") , capmodpp::capmodpp_error );
	EXPECT_NO_THROW( capmodpp::cap_name_to_nr("SYS_ADMIN") );
}


TEST( privileges , create_print_capmodpp ) {
	capmodpp::cap_state_map state_map1;
	_note("State map empty: " << state_map1 );

	auto slashproc1 = detail::read_slash_proc_rights();
	for (const auto & item : slashproc1) _note(item.first << " = " << std::hex << item.second);
	// state at start:
	// TODO: this test can change one day
	EXPECT_EQ( slashproc1.at("CapEff")  ,  0x0000000000001000 );
	EXPECT_EQ( slashproc1.at("CapPrm")  ,  0x0000000000001000 );

	capmodpp::cap_state_map state_map2 = capmodpp::read_process_caps();
	_note("State map loaded:" << state_map2);
}

// g_dbg_level_set(0,"writting a test",false);

TEST( privileges , drop_one_cap ) {
	// our lib reads state:
	capmodpp::cap_state_map state_map = capmodpp::read_process_caps();
	_note("State map before: " << state_map );
  // we have that needed cap:
	EXPECT_EQ( state_map.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).permit , capmodpp::cap_perm::yes );
	// example cap we will not have
	EXPECT_EQ( state_map.state.at( capmodpp::cap_name_to_nr("MAC_OVERRIDE") ).permit , capmodpp::cap_perm::no );

	capmodpp::cap_statechange_full change;
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_all_others({capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});

	/*
	// TODO@rfree - other format, test it when we can run several tests
	change.given.state[  capmodpp::cap_name_to_nr("NET_ADMIN") ]
		.set( capmodpp::cap_area_type::eff , capmodpp::cap_permchange::disable )
		.set( capmodpp::cap_area_type::permit , capmodpp::cap_permchange::disable )
		.set( capmodpp::cap_area_type::inherit , capmodpp::cap_permchange::disable )
	;
	*/
	_mark("Will apply change:" << change);

	change.security_apply_now(); // apply this to process

  // removed:
	capmodpp::cap_state_map state_map2 = capmodpp::read_process_caps();
	_note("State map after: " << state_map2 );

	EXPECT_EQ( state_map2.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).eff ,    capmodpp::cap_perm::no );
	EXPECT_EQ( state_map2.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).permit , capmodpp::cap_perm::no );
	EXPECT_EQ( state_map2.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).inherit, capmodpp::cap_perm::no );

	/*
	change.all_others
		.set( capmodpp::cap_area_type::eff , capmodpp::cap_permchange::unchanged )
		.set( capmodpp::cap_area_type::permit , capmodpp::cap_permchange::unchanged )
		.set( capmodpp::cap_area_type::inherit , capmodpp::cap_permchange::unchanged )
	;
	*/
}

TEST( privileges , drop_all_cap_except_one ) {

#if 0
// TODO@rfree - need to test from start in a new process?

	// our lib reads state:
	capmodpp::cap_state_map state_map = capmodpp::read_process_caps();
	_note("State map before: " << state_map );
  // we have that needed cap:
	EXPECT_EQ( state_map.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).permit , capmodpp::cap_perm::yes );
	// example cap we will not have
	EXPECT_EQ( state_map.state.at( capmodpp::cap_name_to_nr("MAC_OVERRIDE") ).permit , capmodpp::cap_perm::no );

	capmodpp::cap_statechange_full change;
	change.given.state[  capmodpp::cap_name_to_nr("NET_ADMIN") ]
		.set( capmodpp::cap_area_type::eff , capmodpp::cap_permchange::enable )
		.set( capmodpp::cap_area_type::permit , capmodpp::cap_permchange::enable )
		.set( capmodpp::cap_area_type::inherit , capmodpp::cap_permchange::unchanged )
	;
	change.all_others
		.set( capmodpp::cap_area_type::eff , capmodpp::cap_permchange::disable )
		.set( capmodpp::cap_area_type::permit , capmodpp::cap_permchange::disable )
		.set( capmodpp::cap_area_type::inherit , capmodpp::cap_permchange::disable )
	;
	_mark("Will apply change:" << change);

	change.security_apply_now(); // apply this to process

  // removed:
	capmodpp::cap_state_map state_map2 = capmodpp::read_process_caps();
	_note("State map after: " << state_map2 );

	EXPECT_EQ( state_map2.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).eff ,    capmodpp::cap_perm::yes );
	EXPECT_EQ( state_map2.state.at( capmodpp::cap_name_to_nr("NET_ADMIN") ).permit , capmodpp::cap_perm::yes );
#endif

}


#endif

