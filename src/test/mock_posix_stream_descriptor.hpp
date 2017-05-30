#ifndef MOCK_POSIX_STREAM_DESCRIPTOR_HPP
#define MOCK_POSIX_STREAM_DESCRIPTOR_HPP

#include "../../depends/googletest/googlemock/include/gmock/gmock.h"
#include <boost/asio.hpp>
#include <functional>
#include <libs0.hpp>

namespace mock {

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

class mock_posix_stream_descriptor {
	public:
		mock_posix_stream_descriptor(boost::asio::io_service & io_service,
		                             const int &native_descriptor);
	MOCK_METHOD0(is_open, bool());
	MOCK_METHOD0(release, int());
	MOCK_METHOD1(assign, void(const int &));
	MOCK_METHOD1(write_some, size_t(const boost::asio::const_buffers_1 &));
	MOCK_METHOD2(write_some, size_t(std::array<boost::asio::const_buffer, 4>& , boost::system::error_code &));
	MOCK_METHOD1(read_some, size_t(const boost::asio::mutable_buffers_1 &));
	MOCK_METHOD1(read_some, size_t(const std::array<boost::asio::mutable_buffer, 4>&));
	MOCK_METHOD2(async_read_some, void(const boost::asio::mutable_buffer &,
	                                   std::function<void(const boost::system::error_code&, size_t)>));
	static bool s_good_open;
};

} // namespace

#endif // MOCK_POSIX_STREAM_DESCRIPTOR_HPP
