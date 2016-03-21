#ifndef C_IP46_ADDR_H
#define C_IP46_ADDR_H

#include "libs1.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>

// TODO to lib "ip46"

// sockaddr is the base class for sockaddr_in and for sockaddr_in6
// see also http://stackoverflow.com/a/18579605
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in) , "Invalid size of ipv4 vs ipv6 addresses" );
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in6) , "Invalid size of ipv4 vs ipv6 addresses" );

class c_ip46_addr { ///< any address ipv6 or ipv4, in system socket format
	public:
		typedef enum { tag_none, tag_ipv4, tag_ipv6 } t_tag; ///< possible address type

		c_ip46_addr();

		void set_ip4(sockaddr_in in4);
		void set_ip6(sockaddr_in6 in6);
		sockaddr_in  get_ip4() const;
		sockaddr_in6 get_ip6() const;

		t_tag get_ip_type() const;

		static c_ip46_addr any_on_port(int port); ///< return my address, any IP (e.g. for listening), on given port. it should listen on both ipv4 and 6
		static c_ip46_addr create_ipv4(const std::string &ipv4_str, int port);
		static c_ip46_addr create_ipv6(const std::string &ipv6_str, int port);

		/**
		 * @param ipstr string contain ipv4 or ipv6
		 * @return true if ipstr is ipv4, false if ipv6
		 * @throw std::invalid_argument if ipstr is unknown address format
		 * Exception safety: strong exception guarantee
		 */
		static bool is_ipv4(const std::string &ipstr);
		friend ostream &operator << (ostream &out, const c_ip46_addr& addr);

	private:
		struct t_ip_data {
			union { ///< the address is either:
				sockaddr_in in4;
				sockaddr_in6 in6;
			};
		};

		t_ip_data m_ip_data;

		t_tag m_tag; ///< current type of address
};

#endif // C_IP46_ADDR_H
