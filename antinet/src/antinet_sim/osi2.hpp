#ifndef OSI2_HPP
#define OSI2_HPP

#include "libs1.hpp"
#include "c_networld.hpp"
#include "c_osi3_uuid_generator.hpp"

class c_osi2_switch;
class c_osi2_cable_direct;
class c_osi2_nic;
class c_osi2_switch;
class c_networld;

typedef std::string t_osi2_data; // some kind of packet of data sent over OSI2

/*** 
 * @brief Classical cable in OSI layer 2, connecting 2 end point stations (e.g. computer to computer 
 * or computer to switch, or switch to switch)
 */
class c_osi2_cable_direct {
	private:
		std::array< std::reference_wrapper<c_osi2_nic>, 2 > m_endpoint; ///< array of the 2 endpoints; as reference
//		std::array< c_osi2_nic*, 2 > m_endpoint; ///< array of the 2 endpoints; as reference
	public:
		// 
		// c_osi2_cable_direct(c_osi2_nic *a, c_osi2_nic *b);
		c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b);
};




/***
 * @brief a plug that is connected to a cable; That is needed for C++ reasons,
 * to hold a reference to the cable (the reference can not be re-seated, but
 * entire object of this class can be recreated with ref to other cable when
 * needed) 
 **/
class c_osi2_cable_direct_plug {
	private:
		c_osi2_cable_direct & m_cable;
	public:
		c_osi2_cable_direct_plug(c_osi2_cable_direct & cable);
		friend class c_osi2_nic; ///< friend class, so it can take the .m_cable
};





/***
 * @brief The network card for OSI layer 2.
 * It can store data that is being sent in outbox[]
 */
class c_osi2_nic {
	private:
		std::vector< t_osi2_data > m_outbox; ///< the data that is being sent out in OSI2
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		t_osi3_uuid m_osi3_uuid; ///< the UUID that imitates some unique "IP address" of this card (e.g. like internet IP)
		
		unique_ptr<c_osi2_cable_direct_plug> m_plug; ///< the (plug to the) cable to my physical-peer
		
		// t_osi3_addr m_addr; ///< my address in OSI3 (in LAN, in ICANN internet) // TODO remove
	public:
		c_osi2_nic(c_networld &networld);
		
		void plug_in_cable(c_osi2_cable_direct & cable); ///< attach this cable to me
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_osi2_nic &obj);
};





// Classical switch in OSI layer 2
class c_osi2_switch {
	private:
		std::vector<c_osi2_nic> m_nic; ///< all my NIC cards, for all my ports
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
	public:
		c_osi2_switch();
		
		void connect_with(c_osi2_nic &target, c_networld &networld); ///< add port, connect to target, inside networld
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_osi2_switch &obj);
		
};

#endif // OSI2_HPP
