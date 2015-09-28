#ifndef OSI2_HPP
#define OSI2_HPP

#include "libs1.hpp"
#include "c_world.hpp"
#include "c_osi3_uuid_generator.hpp"
#include "c_object.hpp"

class c_world; // world for simulation

class c_osi2_switch;
//class c_osi2_cable_direct;
class c_osi2_nic;
//class c_entity;

typedef std::string t_osi2_data; ///< some kind of packet of data sent over OSI2 (the same is used for OSI3)

struct t_osi3_packet { ///< some packet with data that is travelig over OSI2
	t_osi3_uuid m_dst; ///< the destination of this packet
	t_osi3_uuid m_src; ///< the source of this packet
	t_osi2_data m_data; ///< the data
};

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
		std::array< std::reference_wrapper<c_osi2_nic>, 2 > get_endpoints() const;
		void draw_allegro (c_drawtarget &drawtarget, c_layer &layer_any) const;
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
 * 
 * This card is always plugged into some switch, therefore it can route
 * OSI3 packets.
 */
class c_osi2_nic {
	private:
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		c_osi2_switch & m_switch; ///< the switch in which I am, it will route for me
		
		t_osi3_uuid m_osi3_uuid; ///< the UUID that imitates some unique "IP address" of this card (e.g. like internet IP)
		unique_ptr<c_osi2_cable_direct_plug> m_plug; ///< the (plug to the) cable to my physical-peer
		std::vector< t_osi3_packet > m_outbox; ///< the data that is being sent out to OSI3 (over OSI2)
		
		// t_osi3_addr m_addr; ///< my address in OSI3 (in LAN, in ICANN internet) // TODO remove
	public:
		c_osi2_nic(c_osi2_switch & my_switch); ///< create me as card in this switch
		/// (you still have to put this object into the switch)
		
		c_osi2_nic(const c_osi2_nic &) = delete; ///< copy constructor
		c_osi2_nic& operator = (const c_osi2_nic &)  = delete;
		
		c_osi2_nic(c_osi2_nic &&) = default; ///< move constructor
		c_osi2_nic& operator = (c_osi2_nic &&)  = default;
		
		void plug_in_cable(c_osi2_cable_direct & cable); ///< attach this cable to me
		
		void add_to_outbox(t_osi3_uuid dst, t_osi2_data &&data); ///< sends the packet over network
		
		bool empty_outbox() const;
		
		void print(std::ostream &os) const;
		
		long int get_serial_number() const;
		
		friend std::ostream& operator<<(std::ostream &os, const c_osi2_nic &obj);
		
		c_osi2_switch &get_my_switch() const;
};


#endif // OSI2_HPP
