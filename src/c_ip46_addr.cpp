// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "c_ip46_addr.hpp"
#include <libs0.hpp>


// ==================================================================
// LINUX version
// ==================================================================

#if defined (__linux__)
#include "cpputils.hpp"
#include <netdb.h>

c_ip46_addr::c_ip46_addr() : m_tag(tag_none) { }

c_ip46_addr::t_tag c_ip46_addr::get_ip_type() const {
	return m_tag;
}

c_ip46_addr::c_ip46_addr(const std::string &ip_addr, int port) {
	// ports-TODO(r) also parse the port here
	// TODO parsing ipv6
	if (is_ipv4(ip_addr)) {
		(*this) = create_ipv4(ip_addr, port);
	} else {
		(*this) = create_ipv6(ip_addr, port);
	}
	pfp_dbg1("Parsing ip46 from string ["<<ip_addr<<"], with port=" << port << "] created: " << (*this));
}

void c_ip46_addr::set_ip4(sockaddr_in in4) {
	pfp_assert(in4.sin_family == AF_INET);
	m_tag = tag_ipv4;
	this->m_ip_data.in4 = in4;
}
void c_ip46_addr::set_ip6(sockaddr_in6 in6) {
	pfp_assert(in6.sin6_family == AF_INET6);
	m_tag = tag_ipv6;
	this->m_ip_data.in6 = in6;
}

sockaddr_in  c_ip46_addr::get_ip4() const {
	pfp_assert(m_tag == tag_ipv4);
	auto ret = this->m_ip_data.in4;
	pfp_assert(ret.sin_family == AF_INET);
	return ret;
}
sockaddr_in6 c_ip46_addr::get_ip6() const {
	pfp_assert(m_tag == tag_ipv6);
	auto ret = this->m_ip_data.in6;
	pfp_assert(ret.sin6_family == AF_INET6);
	return ret;
}

///< return my address, any IP (e.g. for listening), on given port
c_ip46_addr c_ip46_addr::any_on_port(int port) {
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	addr_in.sin_addr.s_addr = INADDR_ANY;
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

c_ip46_addr c_ip46_addr::create_ipv4(const string &ipv4_str, int port) {
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	inet_pton(AF_INET, ipv4_str.c_str(), &(addr_in.sin_addr));
	addr_in.sin_port = htons(port);
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

c_ip46_addr c_ip46_addr::create_ipv6(const string &ipv6_str, int port) {
	as_zerofill <sockaddr_in6> addr_in6;
	addr_in6.sin6_family = AF_INET6;
	inet_pton(AF_INET6, ipv6_str.c_str(), &(addr_in6.sin6_addr));
	addr_in6.sin6_port = htons(port);
	c_ip46_addr ret;
	ret.set_ip6(addr_in6);
	return ret;
}

int c_ip46_addr::get_assigned_port() const {
	if (m_tag == tag_ipv4) {
		return ntohs(m_ip_data.in4.sin_port);
	} else if (m_tag == tag_ipv6) {
		return ntohs(m_ip_data.in6.sin6_port);
	}
	//assert(false && "c_ip46_addr has bad ip type (can not return it's port)");
	pfp_warn("Trying to read port on invalid address");
	return 0;
}


bool c_ip46_addr::is_ipv4(const string &ipstr) {
	as_zerofill< addrinfo > hint;
	struct addrinfo *result = nullptr;
	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	int ret = getaddrinfo(ipstr.c_str(), nullptr, &hint, &result);
	if (ret) pfp_throw_error( invalid_argument( join_string_sep("unknown address format, ret",ret,"for ipstr",ipstr)));
	if (!result) pfp_throw_error( invalid_argument( join_string_sep("unknown address format, pointer result",result,"for ipstr",ipstr)));
	auto result_deleter = [&](struct addrinfo *result){ if (!result) pfp_throw_error(runtime_error("NULL in freeaddrinfo")); freeaddrinfo(result); };
	std::unique_ptr<struct addrinfo, decltype(result_deleter)> result_ptr(result, result_deleter);

	if(result_ptr->ai_family == AF_INET) {
		return true;
	}
	else if (result_ptr->ai_family == AF_INET6) {
		return false;
	}
	pfp_assert(false);
}


ostream & operator<<(ostream &out, const c_ip46_addr& addr) {
	if (addr.m_tag == c_ip46_addr::tag_ipv4) {
		char addr_str[INET_ADDRSTRLEN];
		auto ip4_address = addr.get_ip4();
		inet_ntop(AF_INET, &ip4_address.sin_addr, addr_str, INET_ADDRSTRLEN);
		out << addr_str << ":" << addr.get_assigned_port();
	}
	else if (addr.m_tag == c_ip46_addr::tag_ipv6) {
		char addr_str[INET6_ADDRSTRLEN];
		auto ip6_address = addr.get_ip6();
		inet_ntop(AF_INET6, &ip6_address.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		out << "[" << addr_str << "]:" << addr.get_assigned_port();
	}
	else {
		out << "none";
	}
	return out;
}

bool c_ip46_addr::operator==(const c_ip46_addr &rhs) const {
	if (this->m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("lhs: m_tag == tag_none") );
	}
	if (rhs.m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("rhs: m_tag == tag_none") );
	}
	if (this->m_tag != rhs.m_tag) {
		return false;
	}
	if (this->m_tag == t_tag::tag_ipv4) {
		pfp_assert(rhs.m_tag == t_tag::tag_ipv4); // to be sure both this and rhs union data can be accessed as ipv4
		return (!memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr)))
			&& (this->get_assigned_port() == rhs.get_assigned_port());
	}
	if (this->m_tag == t_tag::tag_ipv6) {
		pfp_assert(rhs.m_tag == t_tag::tag_ipv6); // to be sure both this and rhs union data can be accessed as ipv6
		return !memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in6_addr))
			&& (this->get_assigned_port() == rhs.get_assigned_port());
	}
	else {
		pfp_throw_error(std::runtime_error("Unknown IP type"));
		return false;
	}
}

bool c_ip46_addr::operator<(const c_ip46_addr &rhs) const {
	if (this->m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("lhs: m_tag == tag_none") );
	}
	if (rhs.m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("rhs: m_tag == tag_none") );
	}
	if (this->m_tag == t_tag::tag_ipv4 && rhs.m_tag == t_tag::tag_ipv6) {
		return true;
	}
	else if (this->m_tag == t_tag::tag_ipv6 && rhs.m_tag == t_tag::tag_ipv4) {
		return false;
	}

	if (this->m_tag == t_tag::tag_ipv4) {
		pfp_assert(rhs.m_tag == t_tag::tag_ipv4); // to be sure both this and rhs union data can be accessed as ipv4
		auto cmp = memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr));
		if (cmp<0) return true;
		if (cmp>0) return false;
		return this->get_assigned_port() < rhs.get_assigned_port();
	}
	else if (this->m_tag == t_tag::tag_ipv6) {
		pfp_assert(rhs.m_tag == t_tag::tag_ipv6); // to be sure both this and rhs union data can be accessed as ipv6
		auto cmp = memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in_addr));
		if (cmp<0) return true;
		if (cmp>0) return false;
		return this->get_assigned_port() < rhs.get_assigned_port();
	}
	else return false;
}
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MACH__)
// ==================================================================
// BOOST version: WINDOWS / MACOSX
// ==================================================================

#include <iostream>
c_ip46_addr::c_ip46_addr(const std::string &ip_addr, int port)
:
    m_address(boost::asio::ip::address::from_string(ip_addr)),
    m_port(port)
{
}

c_ip46_addr::t_tag c_ip46_addr::get_ip_type() const {
	if (m_address.is_v4()) return t_tag::tag_ipv4;
	else if (m_address.is_v6()) return t_tag::tag_ipv6;
	else return t_tag::tag_none;
}

std::ostream & operator<<(std::ostream &out, const c_ip46_addr& addr_46) {
	auto const & addr = addr_46.m_address;
	if (addr.is_v6()) {
		out << "[" << addr.to_string() << "]:" << addr_46.m_port;
	}
	else {
		out << addr.to_string() << ":" << addr_46.m_port;
		if (! addr.is_v4()) pfp_warn("Unknown IP type, not v4 not v6: " << addr.to_string());
	}
	return out;
}

bool c_ip46_addr::operator==(const c_ip46_addr &rhs) const {
	return (this->m_address == rhs.m_address) && (this->m_port == rhs.m_port);
}

bool c_ip46_addr::operator<(const c_ip46_addr &rhs) const {
	if (this->m_address < rhs.m_address) return true;
	if (this->m_address > rhs.m_address) return false;
	return this->m_port < rhs.m_port;
}

int c_ip46_addr::get_assigned_port() const {
	return m_port;
}

boost::asio::ip::address c_ip46_addr::get_address() const {
	return m_address;
}

void c_ip46_addr::set_address(const boost::asio::ip::address &address) {
	m_address = address;
}

void c_ip46_addr::set_port(int new_port) {
	m_port = new_port;
}
#endif

#if defined(ANTINET_netbsd)
#include "cpputils.hpp"
#include <netdb.h>

c_ip46_addr::c_ip46_addr() : m_tag(t_tag::tag_none) { }

c_ip46_addr::t_tag c_ip46_addr::get_ip_type() const {
	if (m_address.is_v4()) return t_tag::tag_ipv4;
	else if (m_address.is_v6()) return t_tag::tag_ipv6;
	else return t_tag::tag_none;
}

c_ip46_addr::c_ip46_addr(const std::string &ip_addr, int port) {
	// ports-TODO(r) also parse the port here
	// TODO parsing ipv6
	if (is_ipv4(ip_addr)) {
		(*this) = create_ipv4(ip_addr, port);
	} else {
		(*this) = create_ipv6(ip_addr, port);
	}
	pfp_dbg1n("Parsing ip46 from string ["<<ip_addr<<"], with port="<<port<<"] created: "<<(*this));
}

void c_ip46_addr::set_ip4(sockaddr_in in4) {
	assert(in4.sin_family == AF_INET);
	m_tag = t_tag::tag_ipv4;
	this->m_ip_data.in4 = in4;
}
void c_ip46_addr::set_ip6(sockaddr_in6 in6) {
	assert(in6.sin6_family == AF_INET6);
	m_tag = t_tag::tag_ipv6;
	this->m_ip_data.in6 = in6;
}

sockaddr_in  c_ip46_addr::get_ip4() const {
	assert(m_tag == t_tag::tag_ipv4);
	auto ret = this->m_ip_data.in4;
	assert(ret.sin_family == AF_INET);
	return ret;
}

sockaddr_in6 c_ip46_addr::get_ip6() const {
	assert(m_tag == t_tag::tag_ipv6);
	auto ret = this->m_ip_data.in6;
	assert(ret.sin6_family == AF_INET6);
	return ret;
}
 
///< return my address, any IP (e.g. for listening), on given port
c_ip46_addr c_ip46_addr::any_on_port(int port) {
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	addr_in.sin_addr.s_addr = INADDR_ANY;
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

c_ip46_addr c_ip46_addr::create_ipv4(const string &ipv4_str, int port) {
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	inet_pton(AF_INET, ipv4_str.c_str(), &(addr_in.sin_addr));
	addr_in.sin_port = htons(port);
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

c_ip46_addr c_ip46_addr::create_ipv6(const string &ipv6_str, int port) {
	as_zerofill <sockaddr_in6> addr_in6;
	addr_in6.sin6_family = AF_INET6;
	inet_pton(AF_INET6, ipv6_str.c_str(), &(addr_in6.sin6_addr));
	addr_in6.sin6_port = htons(port);
	c_ip46_addr ret;
	ret.set_ip6(addr_in6);
	return ret;
}

int c_ip46_addr::get_assigned_port() const {
	if (m_tag == t_tag::tag_ipv4) {
		return ntohs(m_ip_data.in4.sin_port);
	} else if (m_tag == t_tag::tag_ipv6) {
		return ntohs(m_ip_data.in6.sin6_port);
	}
	//assert(false && "c_ip46_addr has bad ip type (can not return it's port)");
	pfp_warnn("Trying to read port on invalid address");
	return 0;
}
 
bool c_ip46_addr::is_ipv4(const string &ipstr) {
	as_zerofill< addrinfo > hint;
	struct addrinfo *result = nullptr;
	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;
 
	int ret = getaddrinfo(ipstr.c_str(), nullptr, &hint, &result);
	if (ret) pfp_throw_error( invalid_argument( join_string_sep("unknown address format, ret",ret,"for ipstr",ipstr)));
	if (!result) pfp_throw_error( invalid_argument( join_string_sep("unknown address format, pointer result",result,"for ipstr",ipstr)));
	auto result_deleter = [&](struct addrinfo *result){ if (!result) pfp_throw_error(runtime_error("NULL in freeaddrinfo")); freeaddrinfo(result); };
	std::unique_ptr<struct addrinfo, decltype(result_deleter)> result_ptr(result, result_deleter);

	if(result_ptr->ai_family == AF_INET) {
		return true;
	}
	else if (result_ptr->ai_family == AF_INET6) {
		return false;
	}
	assert(false);
}

ostream & operator<<(ostream &out, const c_ip46_addr& addr) {
	if (addr.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
		char addr_str[INET_ADDRSTRLEN];
		auto ip4_address = addr.get_ip4();
		inet_ntop(AF_INET, &ip4_address.sin_addr, addr_str, INET_ADDRSTRLEN);
		out << "[in4] " << addr_str << ":" << addr.get_assigned_port();
	}
	else if (addr.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
		char addr_str[INET6_ADDRSTRLEN];
		auto ip6_address = addr.get_ip6();
		inet_ntop(AF_INET6, &ip6_address.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		out << "[in6] " << addr_str << ":" << addr.get_assigned_port();
	}
	else {
		out << "[none] IP type not recognized";
	}
	return out;
}

bool c_ip46_addr::operator==(const c_ip46_addr &rhs) const {
	if (this->m_tag == c_ip46_addr::t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("lhs: m_tag == tag_none") );
	}
	if (rhs.m_tag == c_ip46_addr::t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("rhs: m_tag == tag_none") );
	}
	if (this->m_tag != rhs.m_tag) {
		return false;
	}
	if (this->m_tag == c_ip46_addr::t_tag::tag_ipv4) {
		assert(rhs.m_tag == t_tag::tag_ipv4); // to be sure both this and rhs union data can be accessed as ipv4
		return (!memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr)))
			&& (this->get_assigned_port() == rhs.get_assigned_port());
	}
	if (this->m_tag == c_ip46_addr::t_tag::tag_ipv6) {
		assert(rhs.m_tag == t_tag::tag_ipv6); // to be sure both this and rhs union data can be accessed as ipv6
		return !memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in6_addr))
			&& (this->get_assigned_port() == rhs.get_assigned_port());
	}
	else {
		pfp_throw_error(std::runtime_error("Unknown IP type"));
		return false;
	}
}

bool c_ip46_addr::operator<(const c_ip46_addr &rhs) const {
	if (this->m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("lhs: m_tag == tag_none") );
	}
	if (rhs.m_tag == t_tag::tag_none) {
		pfp_throw_error( std::invalid_argument("rhs: m_tag == tag_none") );
	}
	if (this->m_tag == t_tag::tag_ipv4 && rhs.m_tag == t_tag::tag_ipv6) {
		return true;
	}
	else if (this->m_tag == t_tag::tag_ipv6 && rhs.m_tag == t_tag::tag_ipv4) {
		return false;
	}

	if (this->m_tag == t_tag::tag_ipv4) {
		assert(rhs.m_tag == t_tag::tag_ipv4); // to be sure both this and rhs union data can be accessed as ipv4
		auto cmp = memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr));
		if (cmp<0) return true;
		if (cmp>0) return false;
		return this->get_assigned_port() < rhs.get_assigned_port();
	}
	else if (this->m_tag == t_tag::tag_ipv6) {
		assert(rhs.m_tag == t_tag::tag_ipv6); // to be sure both this and rhs union data can be accessed as ipv6
		auto cmp = memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in_addr));
		if (cmp<0) return true;
		if (cmp>0) return false;
		return this->get_assigned_port() < rhs.get_assigned_port();
	}
	else return false;
}

boost::asio::ip::address c_ip46_addr::get_address() const {
	return m_address;
}

void c_ip46_addr::set_address(const boost::asio::ip::address &address) {
	m_address = address;
}

void c_ip46_addr::set_port(int new_port) {
	m_port = new_port;
}
#endif
