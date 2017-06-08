

#include "my_tests_lib.hpp"

#include "../tnetdbg.hpp"
#include "../utils/privileges.hpp"
#include "../utils/capmodpp.hpp"

#include <exception>
#include <cmath>
#include <type_traits>

using namespace std;

TEST( privileges , create_print_capmodpp ) {

	g_dbg_level_set(0,"writting a test",false);

	capmodpp::cap_state_map state_map;
	_note("State map: " << state_map );

	EXPECT_EQ(2,2);

}

