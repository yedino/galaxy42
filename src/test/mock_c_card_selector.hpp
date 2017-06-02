#ifndef MOCK_C_CARD_SELECTOR_BASE_HPP
#define MOCK_C_CARD_SELECTOR_BASE_HPP

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../cable/selector.hpp"

namespace mock {

class mock_c_card_selector final : public c_card_selector_base {
	public:
		mock_c_card_selector(unique_ptr<c_cable_base_addr> && my_addr);
		mock_c_card_selector() = default;
		MOCK_CONST_METHOD1(print, void(ostream &));
		MOCK_CONST_METHOD0(get_kind, t_cable_kind());
		MOCK_METHOD0(get_my_addr, c_cable_base_addr &());
		MOCK_CONST_METHOD0(get_my_addr, const c_cable_base_addr &());
};

} // namespace

#endif // MOCK_C_CARD_SELECTOR_BASE_HPP
