#ifndef MOCK_C_ASIOSERVICE_MANAGER_HPP
#define MOCK_C_ASIOSERVICE_MANAGER_HPP

#include "../../depends/googletest/googlemock/include/gmock/gmock.h"
#include <boost/asio.hpp>
#include <libs0.hpp>
#include "../cable/asio_ioservice_manager.hpp"

namespace mock {

class mock_c_asioservice_manager final : public c_asioservice_manager_base {
	public:
		mock_c_asioservice_manager(size_t size);
		MOCK_METHOD1(resize_to_at_least, void(size_t));
		MOCK_METHOD0(get_next_ioservice, boost::asio::io_service &());
		MOCK_CONST_METHOD0(capacity, size_t());
		MOCK_CONST_METHOD0(size, size_t());
		MOCK_METHOD0(stop_all_threadsafe, void());
};

} // namespace

#endif // MOCK_C_ASIOSERVICE_MANAGER_HPP
