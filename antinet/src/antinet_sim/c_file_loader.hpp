#ifndef C_FILE_LOADER_H
#define C_FILE_LOADER_H


#include <fstream>
#include "c_world.hpp"
#include "c_networld.hpp"
#include "c_object.hpp"

/***
 * @author mz-dev@tigusoft.pl
 */
class c_file_loader
{

	c_world * m_world;
public:
	c_file_loader(c_world *world);

	void load(const std::string & p_filename);

	void read_cobject(const std::vector <std::string> &);
	void read_connection(const std::vector <std::string>& );
	void save(const std::string& p_filename);
	void save(std::ostream & file);


//	c_no


};

#endif // C_FILE_LOADER_H
