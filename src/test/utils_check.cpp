#include "gtest/gtest.h"
#include "../utils/check.hpp"
#include <iostream>
#include <exception>

TEST(utils_check, exception_check) {

	EXPECT_THROW({
		_check_user(false);
	}, err_check_user);

	EXPECT_THROW({
		_check_sys(false);
	}, err_check_sys);

	EXPECT_THROW({
		_check_extern(false);
	}, err_check_extern);

	EXPECT_THROW({
		try{
			_check_user(false);
		}catch(const err_check_user_soft&){}
	}, err_check_user);

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

	EXPECT_NO_THROW({
		_check_user(true);
		_check_sys(true);
		_check_extern(true);
	});
}

TEST(utils_check, exception_try) {

	EXPECT_THROW({
		_try_user(false);
	}, err_check_user_soft);

	EXPECT_THROW({
		_try_sys(false);
	}, err_check_sys_soft);

	EXPECT_THROW({
		_try_extern(false);
	}, err_check_extern_soft);

	EXPECT_NO_THROW({
		_try_user(true);
		_try_sys(true);
		_try_extern(true);
		try{
			_try_user(false);
		}catch(const err_check_user&){}
		try{
			_try_sys(false);
		}catch(const err_check_sys&){}
		try{
			_try_extern(false);
		}catch(const err_check_extern&){}
	});
}

TEST(utils_check, check_what_message_test) {

	std::string err("ERROR");
	std::string test_sentence("2+2==5");

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
	}catch(const err_check_user_soft &e){
		std::string msg(e.what());
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
