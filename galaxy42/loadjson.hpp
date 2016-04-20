
#include "jsoncpp/json/json.h"
#include <libs1.hpp>

struct c_peer {
    uint32_t m_nr;		///< peer number (could be usefull in routing)
    std::string m_ip;
    std::string m_public_key;
};

class c_json_file_parser {
  public:
	c_json_file_parser (const std::string &filename);

    c_json_file_parser () = delete;
    c_json_file_parser (const c_json_file_parser &) = delete;

	std::vector<c_peer> get_peers();
	Json::Value get_root () {
		return root;
	}

  private:
    bool parse_file(const std::string &filename);
    Json::Value root;   // will contains the root value after parsing.
};


class c_auth_password_load {
	c_auth_password_load (const std::string &filename) {
  try {
		c_json_file_parser parser(filename);


  } catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
  }
	}

	c_auth_password_load (const c_auth_password_load &) = delete;
};

class c_connect_to_load {
	c_connect_to_load (const std::string &filename) {
  try {
		c_json_file_parser parser(filename);


  } catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
  }
	}

	c_connect_to_load (const c_connect_to_load &) = delete;
};


class c_galaxyconf_load {
  public:
	c_galaxyconf_load (const std::string &filename = "galaxy.conf") {
  try {
		c_json_file_parser parser(filename);


  } catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
  }
	}

	c_galaxyconf_load (const c_galaxyconf_load &) = delete;

  private:


};
