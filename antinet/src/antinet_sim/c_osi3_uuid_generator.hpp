#ifndef C_OSI3_UUID_GENERATOR_HPP
#define C_OSI3_UUID_GENERATOR_HPP

typedef unsigned int t_osi3_uuid; ///< unique address that pretends to be some IP and works in "LAN" and over Internet etc.

/***
 * @brief Use this class to generate the unique UUIDs. E.g. a c_networld could have one of them, then the UUIDs
 * are unique in context of one network world.
 */
class c_osi3_uuid_generator {
	private:
		t_osi3_uuid m_last_uuid; ///< the current position of the generator
	public:
			c_osi3_uuid_generator();
			
			t_osi3_uuid generate();
};

#endif // C_OSI3_UUID_GENERATOR_HPP
