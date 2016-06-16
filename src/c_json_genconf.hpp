#ifndef C_JSON_GENCONF_HPP
#define C_JSON_GENCONF_HPP
#include "libs1.hpp"
#include <boost/filesystem.hpp>

/**
 * @brief The c_json_genconf class
 * 		  class with static members that allow generating different
 * 		  configuration files, using in galaxy network
 */
class c_json_genconf {
  public:
	c_json_genconf() = default;

	/**
	 * @brief genconf
	 *        generate set of default .conf files
	 *        with default names
	 */
	static void genconf ();

  private:
	static void generate_galaxy_conf (const std::string &filename = "galaxy.conf");
	static void generate_authpass_conf (const std::string &filename);
	static void generate_connectto_conf (const std::string &filename);
};

#endif // C_JSON_GENCONF_HPP
