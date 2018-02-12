#include "gtest/gtest.h"
#include "../utils/check.hpp"
#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <memory>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include "my_tests_lib.hpp"

using std::string;

namespace detail {

namespace example_1 { ///< This is a nice demo of throwing various levels of exceptions/asserts

struct example_critical_error { };

enum class orders { do_bad_soft_input, do_bad_soft_sys, do_bad_hard_sys, do_bad_array_index, do_bad_check, do_critical, do_our_assert, do_silly_throw, do_nothing  };
enum class what_happened { nothing_yet, got_soft_input_error, got_soft_error_some_type,
	got_std_exception, got_critical_error , got_unknown_exception , all_fine } ;

// --- demo ---
what_happened start_me(orders order) {
	int size=5;
	if (order == orders::do_bad_soft_input) size=99999;
	_try_input(size < 10);

	int ix=5;
	std::vector<float> vec(100);
	if (order == orders::do_bad_array_index) ix=99999;
	auto val = vec.at(ix);
	_dbg4(val);

	bool file_opened=true;
	if (order == orders::do_bad_soft_sys) file_opened=false;
	_try_sys( file_opened ); // soft error, e.g. this files often file to open

	bool tmp_ok=true;
	if (order == orders::do_bad_hard_sys) tmp_ok=false;
	_check_sys(tmp_ok); // hard error

	int size_half = size/2;
	if (order == orders::do_bad_check) size_half *= 5; // programming error
	_check(size_half <= size);

	// assert()
	if (order == orders::do_our_assert) _check_abort( 2+2 == 5); // will abort here then
	if (order == orders::do_critical) throw example_critical_error();

	if (order == orders::do_silly_throw) throw 42;

	return what_happened::all_fine;
}

what_happened	typical_main_program(orders order) {
	what_happened happened{ what_happened::nothing_yet };
	try {
		try {
			happened = start_me(order); // ***
		}
		catch(const err_check_input &ex) {
			happened = what_happened::got_soft_input_error;
		}
		catch(const err_check_soft &ex) {
			happened = what_happened::got_soft_error_some_type;
		}
		_dbg3("Ok program worked");
		return happened; // program worked despite expected soft problems
	}
	catch(const std::exception &) { happened = what_happened::got_std_exception; }
	catch(const example_critical_error &) { happened = what_happened::got_critical_error; }
	catch(...) { happened = what_happened::got_unknown_exception; } // <-- rethrow if not main()
	return happened;
}
// --- demo ---

} // namespace example_1
} // namespace detail

TEST(utils_check, typical_use_exceptions) {
	using namespace detail::example_1;
	EXPECT_NO_THROW(  typical_main_program( orders::do_nothing )  );
	EXPECT_EQ( typical_main_program( orders::do_nothing ), what_happened::all_fine  );
	EXPECT_EQ( typical_main_program( orders::do_bad_soft_input ) , what_happened::got_soft_input_error  );
	EXPECT_EQ( typical_main_program( orders::do_bad_soft_sys ) , what_happened::got_soft_error_some_type  );

	// no other catch, so std::exception:
	EXPECT_EQ( typical_main_program( orders::do_bad_hard_sys ) , what_happened::got_std_exception  );
	EXPECT_EQ( typical_main_program( orders::do_bad_check ) , what_happened::got_std_exception  );

	EXPECT_EQ( typical_main_program( orders::do_bad_array_index) , what_happened::got_std_exception  );
	EXPECT_EQ( typical_main_program( orders::do_critical) , what_happened::got_critical_error  );
	EXPECT_EQ( typical_main_program( orders::do_silly_throw) , what_happened::got_unknown_exception  );
	EXPECT_EQ( typical_main_program( orders::do_bad_soft_input) , what_happened::got_soft_input_error  );
}

TEST(utils_check, typical_use_abort) {
	using namespace detail::example_1;
	EXPECT_DEATH( typical_main_program( orders::do_our_assert ) , ""); // program will abort
}

TEST(utils_check, exception_check) {

	EXPECT_THROW({
		_check_user(false);
	}, err_check_user);

	EXPECT_THROW({
		_check_input(false);
	}, err_check_input);

	EXPECT_THROW({
		_check_sys(false);
	}, err_check_sys);

	EXPECT_THROW({
		_check_extern(false);
	}, err_check_extern);

	EXPECT_THROW({
		_check(false);
	}, err_check_prog);

	EXPECT_THROW({
		_check(false);
	}, std::exception);

	EXPECT_THROW({
		_check(false);
	}, std::runtime_error);

	EXPECT_THROW({
		try{
			_check_user(false);
		}catch(const err_check_user_soft&){}
	}, err_check_user);

	EXPECT_THROW({
		try{
			_check_input(false);
		}catch(const err_check_input_soft&){}
	}, err_check_input);

	EXPECT_THROW({
		try{
			_check_sys(false);
		}catch(const err_check_sys_soft&){}
	}, err_check_sys);

	EXPECT_THROW({
		try{
			_check_extern(false);
		}catch(const err_check_extern_soft&){}
	}, err_check_extern);

	EXPECT_THROW({
		try{
			_check(false);
			_check_user(false);
			_check_input(false);
			_check_sys(false);
			_check_extern(false);
		}catch(const err_check_soft&){}
	}, std::exception);

	EXPECT_NO_THROW({
		_check(true);
		_check_user(true);
		_check_input(true);
		_check_sys(true);
		_check_extern(true);
	});
}

TEST(utils_check, exception_try) {

	EXPECT_THROW({
		_try_user(false);
	}, err_check_user_soft);

	EXPECT_THROW({
		_try_input(false);
	}, err_check_input_soft);

	EXPECT_THROW({
		_try_sys(false);
	}, err_check_sys_soft);

	EXPECT_THROW({
		_try_extern(false);
	}, err_check_extern_soft);

	EXPECT_NO_THROW({
		_try_user(true);
		_try_input(true);
		_try_sys(true);
		_try_extern(true);
		try{
			_try_user(false);
		}catch(const err_check_user&){}
		try{
			_try_input(false);
		}catch(const err_check_input&){}
		try{
			_try_sys(false);
		}catch(const err_check_sys&){}
		try{
			_try_extern(false);
		}catch(const err_check_extern&){}
		try{
			_try_user(false);
			_try_input(false);
			_try_sys(false);
			_try_extern(false);
		}catch(const err_check_soft&){}
		try{
			_try_user(false);
			_try_input(false);
			_try_sys(false);
			_try_extern(false);
		}catch(const std::exception&){}
	});
}

TEST(utils_check, check_what_message_test) {

	std::string err("ERROR");
	std::string test_sentence("2+2==5");

	try{
		_check(2+2==5);
	}catch(const err_check_prog &e){
		std::string msg(e.what());
		if( msg.find(err) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_check_user(2+2==5);
	}catch(const err_check_user &e){
		std::string msg(e.what());
		if( msg.find(err) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_check_input(2+2==5);
	}catch(const err_check_input &e){
		std::string msg(e.what());
		if( msg.find(err) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_check_sys(2+2==5);
	}catch(const err_check_sys &e){
		std::string msg(e.what());
		if( msg.find(err) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_check_extern(2+2==5);
	}catch(const err_check_extern &e){
		std::string msg(e.what());
		if( msg.find(err) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

}

TEST(utils_check, try_what_message_test) {

	std::string warn("warning");
	std::string test_sentence("2+2==5");

	try{
		_try_user(2+2==5);
	}catch(err_check_soft &e){
		std::string msg(e.what_soft());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_input(2+2==5);
	}catch(err_check_soft &e){
		std::string msg(e.what_soft());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_sys(2+2==5);
	}catch(const err_check_sys_soft &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_extern(2+2==5);
	}catch(const err_check_extern_soft &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}
}

TEST(utils_check, try_cached_as_check_what_message_test) {

	std::string warn("warning");
	std::string test_sentence("2+2==5");

	try{
		_try_user(2+2==5);
	}catch(const err_check_user &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_input(2+2==5);
	}catch(const err_check_input &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_sys(2+2==5);
	}catch(const err_check_sys &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}

	try{
		_try_extern(2+2==5);
	}catch(const err_check_extern &e){
		std::string msg(e.what());
		if( msg.find(warn) == string::npos )
			FAIL();
		if( msg.find(test_sentence) == string::npos )
			FAIL();
	}
}

TEST(utils_check, reasonable_size_string_test) {
	{
		std::string to_big_str(reasonable_size_limit_elements_divided_max+1u, 'a');
		EXPECT_THROW(reasonable_size(to_big_str), err_check_input);
	}
	{
		std::string not_to_big_str(reasonable_size_limit_elements_divided_max, 'a');
		EXPECT_NO_THROW(reasonable_size(not_to_big_str));
	}
}

TEST(utils_check, reasonable_size_containers_test) {
	{
		std::vector<char> too_big_vec(reasonable_size_limit_elements_divided_max+1u);
		EXPECT_THROW(reasonable_size(too_big_vec), err_check_input);
	}
	{
		std::vector<char> not_too_big_vec(reasonable_size_limit_elements_divided_max);
		EXPECT_NO_THROW(reasonable_size(not_too_big_vec));
	}
	{
		size_t size = reasonable_size_limit_bytes_divided_max/sizeof(std::vector<std::array<char, 100u> >::value_type) + 1u;
		std::vector<std::array<char, 100u> > too_big_vec_by_size(size);
		EXPECT_THROW(reasonable_size(too_big_vec_by_size), err_check_input);
	}
	{
		size_t size = reasonable_size_limit_bytes_divided_max/sizeof(std::vector<std::array<char, 100u> >::value_type);
		std::vector<std::array<char, 100u> > not_too_big_vec_by_size(size);
		EXPECT_NO_THROW(reasonable_size(not_too_big_vec_by_size));
	}
	{
		std::multimap<char, char> too_big_map;
		for(size_t i=0; i<reasonable_size_limit_elements_divided_max+1; i++)
		{
			too_big_map.insert(std::make_pair<char, char>('a', 'b'));
		}
		EXPECT_THROW(reasonable_size(too_big_map), err_check_input);
	}
	{
		std::multimap<char, char> not_too_big_map;
		for(size_t i=0; i<reasonable_size_limit_elements_divided_max; i++)
		{
			not_too_big_map.insert(std::make_pair<char, char>('a', 'b'));
		}
		EXPECT_NO_THROW(reasonable_size(not_too_big_map));
	}
	{
		size_t size = reasonable_size_limit_bytes_divided_max/sizeof(std::multimap<char, std::array<long double, 100u> >::value_type) + 1u;
		std::multimap<char, std::array<long double, 100u> > too_big_map_by_size;
		for(size_t i=0; i<size; i++)
		{
			too_big_map_by_size.insert(std::make_pair('a', std::array<long double, 100u>{0}));
		}
		EXPECT_THROW(reasonable_size(too_big_map_by_size), err_check_input);
	}
	{
		size_t size = reasonable_size_limit_bytes_divided_max/sizeof(std::multimap<char, std::array<long double, 100u> >::value_type);
		std::multimap<char, std::array<long double, 100u> > not_too_big_map_by_size;
		for(size_t i=0; i<size; i++)
		{
			not_too_big_map_by_size.insert(std::make_pair('a', std::array<long double, 100u>{0}));
		}
		EXPECT_NO_THROW(reasonable_size(not_too_big_map_by_size));
	}
}

TEST(utils_check, reasonable_size_empty_containers_test) {
	using tup = std::tuple<
		std::vector<char>, std::list<unsigned char>
		,std::set<signed char>, std::multiset<size_t>
		,std::unordered_set<signed char>
		,std::unordered_multiset<size_t>
		,std::map<short, unsigned short>
		,std::multimap<int, unsigned int>
		,std::unordered_map<short, unsigned short>
		,std::unordered_multimap<int, unsigned int>
		,std::queue<int>, std::stack<signed char>
	>;

	tup container_types{};
	for_each(container_types, [](auto empty_container){EXPECT_NO_THROW(reasonable_size(empty_container));});
}
