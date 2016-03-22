#ifndef C_PEERING_H
#define C_PEERING_H

#include "c_ip46_addr.hpp"
#include "haship.hpp"

// TODO: crypto options here
class c_peering { ///< An (mostly established) connection to peer
	public:
		c_peering(const c_ip46_addr & addr_peering , const c_haship_pubkey & pubkey , const c_haship_addr &);

		virtual void send_data(const char * data, size_t data_size)=0;
		virtual ~c_peering()=default;

	protected:
		c_ip46_addr	m_addr; ///< peer address in socket format

		c_haship_pubkey m_pubkey; ///< his pubkey
		c_haship_addr m_haship_addr; ///< his haship address
		// ... TODO crypto type
};

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
		c_peering_udp(const c_ip46_addr & addr_peering , const c_haship_pubkey & pubkey , const c_haship_addr &);

		virtual void send_data(const char * data, size_t data_size);
		virtual void send_data_udp(const char * data, size_t data_size, int udp_socket);
	private:
};

#endif // C_PEERING_H
