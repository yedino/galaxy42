#include "gtest/gtest.h"
#include <tnetdbg.hpp>
#include <iostream>

TEST(UsePtr_Test, abort_test) {
	ASSERT_DEATH(UsePtr(std::unique_ptr<int>(nullptr)), "");
	ASSERT_DEATH(UsePtr(std::unique_ptr<const int>(nullptr)), "");
	ASSERT_DEATH(UsePtr(std::shared_ptr<int>(nullptr)), "");
	ASSERT_DEATH(UsePtr(std::shared_ptr<const int>(nullptr)), "");
}

TEST(UsePtr_Test, dereference_test) {
	auto str_unique_ptr = std::make_unique<std::string>("Test");
	auto str_shared_ptr = std::make_shared<std::string>("Test");
	const auto str_const_unique_ptr = std::make_unique<std::string>("Test");
	const auto str_const_shared_ptr = std::make_shared<std::string>("Test");
	auto const_str_unique_ptr = std::make_unique<const std::string>("Test");
	auto const_str_shared_ptr = std::make_shared<const std::string>("Test");
	const auto const_str_const_unique_ptr = std::make_unique<const std::string>("Test");
	const auto const_str_const_shared_ptr = std::make_shared<const std::string>("Test");
	EXPECT_EQ(UsePtr(str_unique_ptr).at(2), str_unique_ptr->at(2));
	EXPECT_EQ(UsePtr(str_shared_ptr).at(2), str_shared_ptr->at(2));
	EXPECT_EQ(UsePtr(const_str_unique_ptr).at(2), const_str_unique_ptr->at(2));
	EXPECT_EQ(UsePtr(const_str_shared_ptr).at(2), const_str_shared_ptr->at(2));
	EXPECT_EQ(UsePtr(str_const_unique_ptr).at(2), str_const_unique_ptr->at(2));
	EXPECT_EQ(UsePtr(str_const_shared_ptr).at(2), str_const_shared_ptr->at(2));
	EXPECT_EQ(UsePtr(const_str_const_unique_ptr).at(2), const_str_const_unique_ptr->at(2));
	EXPECT_EQ(UsePtr(const_str_const_shared_ptr).at(2), const_str_const_shared_ptr->at(2));
}

TEST(asserts_test, abort_test) {
	ASSERT_DEATH(_assert(2+2==5), "");
	ASSERT_DEATH(_assert(false), "");
	ASSERT_DEATH(_assert(0), "");
	ASSERT_DEATH(_assert(2>3), "");
	ASSERT_DEATH(_check_abort(2+2==5), "");
	ASSERT_DEATH(_check_abort(false), "");
	ASSERT_DEATH(_check_abort(0), "");
	ASSERT_DEATH(_check_abort(2>3), "");
	/*
	_assert(true);
	_assert(1);
	_assert(-1);
	_assert(2+2==4);
	_assert(2<3);
	_check_abort(true);
	_check_abort(1);
	_check_abort(-1);
	_check_abort(2+2==4);
	_check_abort(2<3);
	*/
}
