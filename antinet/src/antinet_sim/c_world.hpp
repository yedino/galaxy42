#ifndef C_WORLD_HPP
#define C_WORLD_HPP

#include <vector>
#include <memory>

#include "c_osi3_uuid_generator.hpp"
#include "osi2.hpp"
#include "c_node.hpp"
#include "c_drawtarget.hpp"
#include "c_object.hpp"
#include "c_network.hpp"

using std::vector;
using std::shared_ptr;

typedef long double t_simclock;

/***
The logical world of the simulation, includes simulated entities.
*/

//class c_osi2_switch;
class c_osi2_cable_direct;
class c_osi2_nic;
class c_file_loader;

/***
 * @author dev-rf@tigusoft.pl
 * @brief The logical world of the simulation, includes simulated entities,
 * also includes the world of the networking (e.g. with UUID/unique global IP addresses space)
 * holds (and usually owns) some objects:
 * - all the cables (no one holds them since they are in between other objects)
 * 
 * Could also hold:
 * - switches and other network devices (that are part of simulation)
 */
class c_world {
	private:
		friend class c_file_loader;
		std::vector< c_osi2_cable_direct > m_cable_direct; ///< all the cables that are hold by this world
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		c_osi3_uuid_generator m_uuid_generator; ///< to generate UUIDs (addresses) in my context
		
		
	public: // TODO?
		vector<unique_ptr<c_object> > m_objects; ///< all the objects in simulation
		
		/***	
		 * clock that dictates flow of the simulated events
		 */
		 t_simclock m_simclock; ///< @TODO wos
		 
		 
		// additional situation in this world:
		shared_ptr<c_tnetdev> m_target, m_source; ///< current source and target used in tests
		
	public:
		c_world();
		
		// building the world - main
		size_t add_osi2_switch(const std::string & name, int x, int y); ///< add switch, returns it's index
		size_t add_node(const std::string & name, int x, int y); ///< add switch, returns it's index
		
		void connect_network_devices(c_object &first, c_object &second); ///< connect the two objects together in network
		void connect_network_devices(size_t nr_a, size_t nr_b); ///< connect by index
		void connect_network_devices(const string & nr_a, const string & nr_b); ///< connect by name
		
		size_t find_object_by_name_as_index(const string & name) const;
		
		// building the world - details:
		c_osi2_cable_direct& new_cable_between(c_osi2_nic &a, c_osi2_nic &b); ///< create a new cable, own it
		
		// misc functions of the world:
		t_osi3_uuid generate_osi3_uuid(); ///< generate a new UUID address inside our world
		
		
		static inline t_simclock get_time();
		static const inline t_simclock get_chronon(){return 0.1;}
	
		// old code, needs reimplementing: @TODO
		void add_test ();
		void tick ();
		void draw(c_drawtarget &drawtarget); ///< tells the world to draw all it's objects
		void load (const string &filename);
		void serialize(ostream &stream);
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_world &obj);
};

ostream &operator<< (ostream &stream, const c_world &world);

#endif // C_WORLD_HPP
