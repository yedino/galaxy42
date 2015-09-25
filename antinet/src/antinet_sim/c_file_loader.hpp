#ifndef C_FILE_LOADER_H
#define C_FILE_LOADER_H


#include "c_world.hpp"
#include "c_networld.hpp"
#include "c_object.hpp"

class c_file_loader
{

	c_world * m_world;
public:
	c_file_loader(c_world *world);

	void load(std::string p_filename);

	void read_cobject(std::vector <std::string> );
	void read_connection(std::vector <std::string> );
	void save(std::string p_filename);


//	c_no


};

#endif // C_FILE_LOADER_H
