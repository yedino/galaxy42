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
			too_big_map_by_size.insert(std::make_pair('a', std::array<long double, 100u>{}));
		}
		EXPECT_THROW(reasonable_size(too_big_map_by_size), err_check_input);
	}
	{
		size_t size = reasonable_size_limit_bytes_divided_max/sizeof(std::multimap<char, std::array<long double, 100u> >::value_type);
		std::multimap<char, std::array<long double, 100u> > not_too_big_map_by_size;
		for(size_t i=0; i<size; i++)
		{
			not_too_big_map_by_size.insert(std::make_pair('a', std::array<long double, 100u>{}));
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
