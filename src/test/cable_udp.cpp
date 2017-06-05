#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../cable/udp/cable_udp_obj.hpp"
#include "mock_c_card_selector.hpp"
#include "mock_c_asioservice_manager.hpp"

using testing::Return;
using testing::_;
using testing::Invoke;

TEST(cable_udp, constructor) {
	std::shared_ptr<c_asioservice_manager_base> empty_ptr;
	mock::mock_c_card_selector card_selector;
	EXPECT_THROW(c_cable_udp(empty_ptr, card_selector), std::invalid_argument);

	std::shared_ptr<c_asioservice_manager_base> asioservice_manage_ptr = std::make_shared<mock::mock_c_asioservice_manager>(1);
	EXPECT_NO_THROW(c_cable_udp(asioservice_manage_ptr, card_selector));
}
