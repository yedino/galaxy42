#include "jsoncpp/json/json.h"
#include <libs1.hpp>

struct c_peer {
    std::string m_ip;
    std::string m_public_key;
};

struct c_auth_password {
	std::string m_password;
	std::string m_myname;
};

class c_json_file_parser {
  public:
	c_json_file_parser (const std::string &filename);

    c_json_file_parser () = delete;
    c_json_file_parser (const c_json_file_parser &) = delete;

	Json::Value get_root ();

  private:
    bool parse_file(const std::string &filename);
	Json::Value m_root;   // will contains the root value after parsing.
};


class c_auth_password_load {
  public:
	c_auth_password_load (const std::string &filename);

	c_auth_password_load () = delete;
	c_auth_password_load (const c_auth_password_load &) = delete;

	std::vector<c_auth_password> get_authpass();
  private:
	std::string m_filename;
	Json::Value m_root;
};

class c_connect_to_load {
  public:
	c_connect_to_load (const std::string &filename);

	c_connect_to_load () = delete;
	c_connect_to_load (const c_connect_to_load &) = delete;

	std::vector<c_peer> get_peers();
  private:
	std::string m_filename;
	Json::Value m_root;
};


class c_galaxyconf_load {
  public:
	c_galaxyconf_load (const std::string &filename = "galaxy.conf");
	c_galaxyconf_load (const c_galaxyconf_load &) = delete;

  private:
	std::string m_filename;
	Json::Value m_root;

	void auth_password_load ();
	void connect_to_load ();
};
