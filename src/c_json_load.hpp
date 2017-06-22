// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "gtest/gtest_prod.h"
#include "libs1.hpp"

#include <json.hpp>
using json = nlohmann::json;

#include "c_json_genconf.hpp"
#include "c_peering.hpp"
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
 * @brief The json_file_parser class
 *        General class for parsing json files to json objects,
 *        it also could be use for checking if json file is correctly formatted  (throw prasing errors)
 */
struct json_file_parser final {

	json_file_parser () = delete;
	json_file_parser (const json_file_parser &) = delete;

	static std::string remove_comments(std::istream &istr);
	static json parse_file(const std::string &filename);
};

class c_auth_password_load final {
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
	json m_json;
	std::string m_filename;
};

class c_connect_to_load final {
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
	json m_json;
	std::string m_filename;
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
	std::vector<t_auth_password> get_auth_passwords ();

  private:
	FRIEND_TEST(json_configfile, load_keys);

	json m_json;
	std::string m_filename;
	std::vector<t_peering_reference> m_peer_references;
	std::vector<t_auth_password> m_auth_passwords;

	t_my_keypair my_keypair_load ();
	void auth_password_load ();
	void connect_to_load ();
};
