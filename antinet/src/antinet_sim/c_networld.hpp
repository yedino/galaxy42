#ifndef C_NETWORLD_HPP
#define C_NETWORLD_HPP

#include "libs1.hpp"
#include "osi2.hpp"
#include "c_osi3_uuid_generator.hpp"

class c_osi2_switch;
class c_osi2_cable_direct;
class c_osi2_nic;
class c_osi2_switch;

/***
 * @brief one world of the networking, holds (and usually owns) some objects:
 * - should hold the cables (no one holds them since they are in between other objects)
 */
class c_networld {
	private:
		std::vector< c_osi2_cable_direct > m_cable_direct; ///< all the cables that are hold by this networld
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		c_osi3_uuid_generator m_uuid_generator; ///< to generate UUIDs (addresses) in my context
		std::map < std::pair < t_osi3_uuid, t_osi3_uuid >, t_osi3_uuid > m_next_hop_map;
	public:
		c_networld();
		c_osi2_cable_direct& get_new_cable_between(c_osi2_nic &a, c_osi2_nic &b); ///< create a new cable, own it, and return ref to it
		
		t_osi3_uuid generate_osi3_uuid();
		
		void generate_next_hop_map();
		t_osi3_uuid get_next_hop(const t_osi3_uuid &source, const t_osi3_uuid &destination);
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_networld &obj);
};

#endif // C_NETWORLD_HPP
