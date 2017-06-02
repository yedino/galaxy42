#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../cable/udp/cable_udp_obj.hpp"
#include "mock_c_card_selector.hpp"

using testing::Return;
using testing::_;
using testing::Invoke;

TEST(cable_udp, constructor) {
	shared_ptr<c_asioservice_manager_base> empty_ptr;
	mock::mock_c_card_selector card_selector;
	EXPECT_ANY_THROW(c_cable_udp(empty_ptr, card_selector));
}
