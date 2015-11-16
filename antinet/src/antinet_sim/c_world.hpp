#ifndef C_WORLD_HPP
#define C_WORLD_HPP

#include <vector>
#include <memory>

#include "c_simulation.hpp"
#include "c_osi3_uuid_generator.hpp"
#include "osi2.hpp"
#include "c_node.hpp"
#include "c_drawtarget.hpp"
#include "c_object.hpp"

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

struct t_osi2_route_result {
		bool valid; ///< all the data is valid. else, data is not valid (route can not be found)
		
		// only if m_valid==1 then you can use this fields:
		
		size_t first_hop_nic_ix; ///< NIC card used at begin: the index of first.m_nic[] on the route
		size_t target_nic_ix; ///< NIC card used at end: the index of second.m_nic[] on the route
		t_osi2_cost cost; ///< totall cost of the route
		
		vector< t_osi3_uuid > hops_uuid; ///< the route as "list" of UUIDs of NIC cards.
};


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
class c_simulation;

class c_world {
	private:
		friend class c_file_loader;
		std::vector< std::unique_ptr<c_osi2_cable_direct> > m_cable_direct; ///< all the cables that are hold by this world
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		c_osi3_uuid_generator m_uuid_generator; ///< to generate UUIDs (addresses) in my context
        c_simulation &m_simulation;

		
	public: // TODO?
		vector<unique_ptr<c_object> > m_objects; ///< all the objects in simulation
        bool get_is_pause();
        /***
		 * clock that dictates flow of the simulated events
		 */
		 t_simclock m_simclock; ///< @TODO wos
		 
		
    public:
        c_world() = delete;
        c_world(c_simulation&);
		
		// building the world - main
		void add_osi2_switch(const std::string & name, int x, int y); ///< add switch, returns it's index
		void add_node(const std::string & name, int x, int y); ///< add switch, returns it's index
		
		/// connect the two objects together in network:
		void connect_network_devices(c_object &first, c_object &second, t_osi2_cost cost); 
		void connect_network_devices(size_t nr_a, size_t nr_b, t_osi2_cost cost); ///< connect by index
		void connect_network_devices(const string & nr_a, const string & nr_b, t_osi2_cost cost); ///< connect by name
		
		
		t_osi2_route_result route_find_route_between(c_object &first, c_object &second);
		
		/***
		 * @return: the ix of card in first to use to route to reach second. first.m_nic[ix] is the card
		 * that will lead to second.
		 */
		size_t_maybe route_next_hop_nic_ix(c_object &first, c_object &second);
		
		c_osi2_switch & object_to_switch(c_object &object) const; ///< try to convert object to switch, throw if not
		
		size_t find_object_by_name_as_index(const string & name) const;
		c_object& find_object_by_name_as_object(const string & name);
		c_osi2_switch& find_object_by_name_as_switch(const string & name);


		c_osi2_switch &find_object_by_uuid_as_switch(const t_osi3_uuid);
		c_object& find_object_by_uuid_as_object(const t_osi3_uuid);
		size_t find_object_by_uuid_as_index(const t_osi3_uuid);

		
		// building the world - details:
		c_osi2_cable_direct& new_cable_between(c_osi2_nic &a, c_osi2_nic &b, t_osi2_cost cost); ///< create a new cable, own it
		
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
