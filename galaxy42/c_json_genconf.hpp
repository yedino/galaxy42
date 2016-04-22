#ifndef C_JSON_GENCONF_HPP
#define C_JSON_GENCONF_HPP
#include "libs1.hpp"

class c_json_genconf {
  public:
	c_json_genconf() = default;

	static void genconf ();

	static void generate_galaxy_conf (const std::string &filename = "galaxy.conf");
	static void generate_authpass_conf (const std::string &filename);
	static void generate_connectto_conf (const std::string &filename);
};

#endif // C_JSON_GENCONF_HPP
