

#include "my_tests_lib.hpp"

#include "../utils/privileges.hpp"
#include "../utils/capmodpp.hpp"

#include <exception>
#include <cmath>
#include <type_traits>


TEST( privileges , create_print_capmodpp ) {

	capmodpp::cap_state_map state_map;

	_goal( state_map );

	EXPECT_EQ(2,2);

}

