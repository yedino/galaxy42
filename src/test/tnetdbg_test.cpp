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

