#include "c_ip46_addr.hpp"

#include "cpputils.hpp"
#include <netdb.h>

c_ip46_addr::c_ip46_addr() : m_tag(tag_none) { }

c_ip46_addr::t_tag c_ip46_addr::get_ip_type() const {
	return m_tag;
}

void c_ip46_addr::set_ip4(sockaddr_in in4) {
	_assert(in4.sin_family == AF_INET);
	m_tag = tag_ipv4;
	this->m_ip_data.in4 = in4;
}
void c_ip46_addr::set_ip6(sockaddr_in6 in6) {
	_assert(in6.sin6_family == AF_INET6);
	m_tag = tag_ipv6;
	this->m_ip_data.in6 = in6;
}

sockaddr_in  c_ip46_addr::get_ip4() const {
	_assert(m_tag == tag_ipv4);
	auto ret = this->m_ip_data.in4;
	_assert(ret.sin_family == AF_INET);
	return ret;
}
sockaddr_in6 c_ip46_addr::get_ip6() const {
	_assert(m_tag == tag_ipv6);
	auto ret = this->m_ip_data.in6;
	_assert(ret.sin6_family == AF_INET6);
	return ret;
}

c_ip46_addr c_ip46_addr::any_on_port(int port) { ///< return my address, any IP (e.g. for listening), on given port
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


bool c_ip46_addr::is_ipv4(const string &ipstr) {
	as_zerofill< addrinfo > hint;
	struct addrinfo *result = nullptr;
	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;
	int ret = getaddrinfo(ipstr.c_str(), nullptr, &hint, &result);
	if (ret) {
		throw std::invalid_argument("unknown address format");
	}
	auto result_deleter = [&](struct addrinfo *result){freeaddrinfo(result);};
	std::unique_ptr<struct addrinfo, decltype(result_deleter)> result_ptr(result, result_deleter);
	if(result_ptr->ai_family == AF_INET) {
		return true;
	}
	else if (result_ptr->ai_family == AF_INET6) {
		return false;
	}
	_assert(false);
}


ostream &operator << (ostream &out, const c_ip46_addr& addr) {
	if (addr.m_tag == c_ip46_addr::tag_ipv4) {
		char addr_str[INET_ADDRSTRLEN];
		auto ip4_address = addr.get_ip4();
		inet_ntop(AF_INET, &ip4_address.sin_addr, addr_str, INET_ADDRSTRLEN);
		out << addr_str;
	}
	else if (addr.m_tag == c_ip46_addr::tag_ipv6) {
		char addr_str[INET6_ADDRSTRLEN];
		auto ip6_address = addr.get_ip6();
		inet_ntop(AF_INET6, &ip6_address.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		out << addr_str;
	}
	else {
		out << "none";
	}
	return out;
}

bool c_ip46_addr::operator== (const c_ip46_addr &rhs) const {
	if (this->m_tag == t_tag::tag_none) {
		throw std::invalid_argument("lhs: m_tag == tag_none");
	}
	if (rhs.m_tag == t_tag::tag_none) {
		throw std::invalid_argument("rhs: m_tag == tag_none");
	}
	if (this->m_tag != rhs.m_tag) {
		return false;
	}
	if (this->m_tag == t_tag::tag_ipv4) {
		return !memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr));
	}
	else {
		return !memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in6_addr));
	}
}

bool c_ip46_addr::operator< (const c_ip46_addr &rhs) const {
	if (this->m_tag == t_tag::tag_none) {
		throw std::invalid_argument("lhs: m_tag == tag_none");
	}
	if (rhs.m_tag == t_tag::tag_none) {
		throw std::invalid_argument("rhs: m_tag == tag_none");
	}
	if (this->m_tag == t_tag::tag_ipv4 && rhs.m_tag == t_tag::tag_ipv6) {
		return true;
	}
	else if (this->m_tag == t_tag::tag_ipv6 && rhs.m_tag == t_tag::tag_ipv4) {
		return false;
	}

	int ret = 0;
	if (rhs.m_tag == t_tag::tag_ipv4) {
		int ret = memcmp(&this->m_ip_data.in4.sin_addr, &rhs.m_ip_data.in4.sin_addr, sizeof(in_addr));
	}
	else {
		int ret = memcmp(&this->m_ip_data.in6.sin6_addr, &rhs.m_ip_data.in6.sin6_addr, sizeof(in6_addr));
	}
	if (ret < 0) return true;
	else return false;
}
