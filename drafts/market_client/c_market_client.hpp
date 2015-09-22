#ifndef C_CLIENT_HPP
#define C_CLIENT_HPP

#include "c_threadguard.hpp"
#include "../../crypto_ops/crypto/c_encryption.hpp"
#include "c_connect.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <memory>

///////////////////////////////////////////////////////////////////////////////////////////////////////////

class c_market_client {
public:
	c_market_client(string host, string port,string ptype);
	~c_market_client();
	void encrypt_client(cryptosign_method crypt_m);
	void start_market_session();
private:
	std::shared_ptr<c_connect> m_market_link;
	std::shared_ptr<c_encryption> cl_crypto;
	//thread_guard link_init_t;
};

#endif // C_OBJECT_HPP

class c_client_user {
public:
	c_client_user() : m_username("non")
	{}
	c_client_user(std::string usr) : m_username(usr)
	{}

	void set_username(std::string usr) {
		m_username = usr;
	}

	std::string get_username() {
		return m_username;
	}

private:
	std::string m_username;
};
