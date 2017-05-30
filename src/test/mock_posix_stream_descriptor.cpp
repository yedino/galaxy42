#include "mock_posix_stream_descriptor.hpp"
#include "../tuntap/linux/c_tuntap_linux_obj.hpp"

bool mock::mock_posix_stream_descriptor::s_good_open = true;

mock::mock_posix_stream_descriptor::mock_posix_stream_descriptor(boost::asio::io_service &io_service, const int &native_descriptor) {
	  _UNUSED(io_service);
	  _UNUSED(native_descriptor);
	  EXPECT_CALL(*this, is_open())
	              .WillRepeatedly(Return(s_good_open));
	  s_good_open = true;
}
