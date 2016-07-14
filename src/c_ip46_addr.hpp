// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef C_IP46_ADDR_H
#define C_IP46_ADDR_H

#if defined(__linux__)

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

class c_ip46_addr { ///< any address ipv6 or ipv4, in lowest level - system socket format
	public:
		typedef enum { tag_none, tag_ipv4, tag_ipv6 } t_tag; ///< possible address type

		c_ip46_addr();
		/// Default port: 9042

		///! Constructs, from ip_addr in format hexdotip e.g.: fd42:ae11:f636:86:5:1:e5c4:d
		c_ip46_addr(const std::string &ip_addr, int port = 9042);

		void set_ip4(sockaddr_in in4);
		void set_ip6(sockaddr_in6 in6);

		sockaddr_in  get_ip4() const;
		sockaddr_in6 get_ip6() const;

		t_tag get_ip_type() const;
		int get_assign_port() const {
			if(m_tag == tag_ipv4) {
				return ntohs(m_ip_data.in4.sin_port);
			} else if (m_tag == tag_ipv6){
				return ntohs(m_ip_data.in6.sin6_port);
			}

			assert(false && "c_ip46_addr has bad ip type");
			return 0;
		}

		/// return my address, any IP (e.g. for listening), on given port.
		/// it should listen on both ipv4 and 6
		static c_ip46_addr any_on_port(int port);
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

		/**
		 * @return false if m_tag of lhs and rhs are different
		 * @throw std::invalid_argument if m_tag of lhs or rhs == tag_none
		 * Exception safety: strong exception guarantee
		 */
		bool operator == (const c_ip46_addr &rhs) const;

		/**
		 * @throw std::invalid_argument if m_tag of lhs or rhs == tag_none
		 * Exception safety: strong exception guarantee
		 */
		bool operator < (const c_ip46_addr &rhs) const;


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
#endif // __linux__

#if defined(_WIN32) || defined(__CYGWIN__)

#include <boost/asio/ip/address.hpp>
#include <ostream>
class c_ip46_addr {
	public:
		typedef enum { tag_none, tag_ipv4, tag_ipv6 } t_tag; ///< possible address type
		c_ip46_addr() = default;
		///! Constructs, from ip_addr in format hexdotip e.g.: fd42:ae11:f636:86:5:1:e5c4:d
		c_ip46_addr(const std::string &ip_addr, int port = 9042);
		t_tag get_ip_type() const;
		friend std::ostream &operator << (std::ostream &out, const c_ip46_addr& addr);
		bool operator == (const c_ip46_addr &rhs) const;
		bool operator < (const c_ip46_addr &rhs) const;
		int get_assign_port() const;
	private:
		boost::asio::ip::address m_address;
		int m_port;
};
#endif

#endif // C_IP46_ADDR_H
