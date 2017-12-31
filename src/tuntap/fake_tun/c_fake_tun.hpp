#ifndef C_FAKE_TUN_HPP
#define C_FAKE_TUN_HPP

#include <boost/asio.hpp>
#include <string>
#include "../base/tuntap_base.hpp"

class c_fake_tun final : public c_tuntap_base_obj {
	public:
		c_fake_tun(boost::asio::io_service &io_service,
			const std::string listen_ipv4_address = "0.0.0.0",
			const short listen_port = 10000
		);

		/**
		 * @brief send_to_tun
		 * @param data pointer to block of data which ipv6 packet
		 * @param size size of block of data pointed by data pointer
		 * @return size of sended data, 0 if error
		 */
		size_t send_to_tun(const unsigned char *data, size_t size) override; ///< blocking function

		/**
		 * @brief send_to_tun_separated_addresses
		 * @param data pointer to block of data which ipv6 packet without src and dst address fields
		 * @param size size of block of data pointed by data pointer
		 * @param src_binary_address source address fiels
		 * @param dst_binary_address destination address field
		 * @return number of written bytes, if success returns size + src_binary_address.size() + dst_binary_address.size(),
		 * returns 0 if error
		 */
	virtual size_t send_to_tun_separated_addresses(const unsigned char * const , size_t ,
			const std::array<unsigned char, IPV6_LEN> &,
			const std::array<unsigned char, IPV6_LEN> &) override {std::abort(); return 0;}

		/**
		 * @brief read_from_tun
		 * @param data pointer to block of data which will be filled by packet must be preallocated
		 * @param size size of block of data pointed by data pointer
		 * @return number of readed bytes, 0 if error
		 */
		virtual size_t read_from_tun(unsigned char * const data, size_t size) override; ///< blocking function

		/**
		 * @brief read_from_tun_separated_addresses
		 * @param data pointer to block of data which will be filled by ipv6 packet without src and dst address fields, must be preallocated
		 * @param size size of block of data pointed by data pointer
		 * @param src_binary_address source address filed
		 * @param dst_binary_address destination address field
		 * @return number of readed bytes without - 32 (dst size + src size == 32) or 0 if error
		 * if 0 is returned in data, src_binary_address or dst_binary_address may be trash data
		 */
		virtual size_t read_from_tun_separated_addresses(unsigned char * const, size_t,
			std::array<unsigned char, IPV6_LEN> &,
			std::array<unsigned char, IPV6_LEN> &) override {std::abort();return 0;}

		// receive data to provided buffer, and then callback. Buffer must be valid up untill callback is called
		// then the callback is allowed to e.g. deallocate (or reuse) it.
		virtual void async_receive_from_tun(unsigned char * const , size_t , const read_handler & ) override{std::abort();};

		// sets the parameters of our tuntap
		void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &, int , uint32_t) override {};
	private:
		boost::asio::ip::udp::socket m_socket;
};

#endif // C_FAKE_TUN_HPP
