#ifndef ASIO_IOSERVICE_MANAGER_HPP
#define ASIO_IOSERVICE_MANAGER_HPP

#include <array>
#include <boost/asio/io_service.hpp>

class asio_ioservice_manager final {
	public:
		static boost::asio::io_service &get_next_ioservice();
	private:
		static std::array<boost::asio::io_service, 5> m_ioservice_array;
		static size_t m_last_ioservice;

};

#endif // ASIO_IOSERVICE_MANAGER_HPP
