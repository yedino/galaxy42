// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once


#include <json/json.h>
#include "c_json_genconf.hpp"
#include "libs1.hpp"
#include "c_peering.hpp"
#include "datastore.hpp"
#include "strings_utils.hpp"

struct t_my_keypair {
	std::string m_private_key_type;
	std::string m_private_key;
	std::string m_public_key;
	std::string m_ipv6;
};

struct t_auth_password {
	std::string m_password;
	std::string m_myname;
};

/**
 * @brief The c_json_file_parser class
 * 		  General class for parsing json files to root Json::Value,
 * 		  it also could be use for checking if json file is correctly formatted
 */
class c_json_file_parser {
  public:
	c_json_file_parser (const std::string &filename);

    c_json_file_parser () = delete;
    c_json_file_parser (const c_json_file_parser &) = delete;

	Json::Value get_root ();

  private:
	bool parse_file (const std::string &filename);
	Json::Value m_root;   // will contains the root value after parsing.
};

class c_auth_password_load {
  public:
	/**
	 * \brief Loading authorized passwords from configuration file
	 * \param filename - configuration file name.
	 * \param auth_passwords - reference to vector of t_auth_password to push_back new authorized password
	 */
	c_auth_password_load (const std::string &filename, std::vector<t_auth_password> &auth_passwords);

	c_auth_password_load () = delete;
	c_auth_password_load (const c_auth_password_load &) = delete;

	void get_auth_passwords (std::vector<t_auth_password> &auth_passwords);
  private:
	std::string m_filename;
	Json::Value m_root;
};

class c_connect_to_load {
  public:
	/**
	 * \brief Loading peer references from configuration file
	 * \param filename - configuration file name.
	 * \param peer_ref - reference to vector of t_peering_reference to push_back new peering references
	 */
	c_connect_to_load (const std::string &filename, std::vector<t_peering_reference> &peer_refs);

	c_connect_to_load () = delete;
	c_connect_to_load (const c_connect_to_load &) = delete;

	void get_peers (std::vector<t_peering_reference> &peer_refs);
  private:
	std::string m_filename;
	Json::Value m_root;
};
/**
 * @brief The c_galaxyconf_load class
 * 		  Main class for parsing and getting all available info from galaxy.conf file
 */
class c_galaxyconf_load {
  public:
	c_galaxyconf_load (const std::string &filename = "galaxy.conf");
	c_galaxyconf_load (const c_galaxyconf_load &) = delete;

	std::vector<t_peering_reference> get_peer_references ();

  private:
	std::string m_filename;
	Json::Value m_root;
	std::vector<t_peering_reference> m_peer_references;
	std::vector<t_auth_password> m_auth_passwords;

	t_my_keypair my_keypair_load ();
	void auth_password_load ();
	void connect_to_load ();
};
