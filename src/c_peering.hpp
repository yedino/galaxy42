// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef C_PEERING_H
#define C_PEERING_H

#include <atomic>

#include "formats_ip.hpp"
#include "c_ip46_addr.hpp"
#include "haship.hpp"
#include "protocol.hpp"

#include "crypto/crypto_basic.hpp"
#include "c_udp_wrapper.hpp"

// TODO (later) make normal virtual functions (move UDP properties into class etc) once tests are done.

struct t_peering_reference {
	public:
		t_peering_reference(const t_ipv46dot & peering_addr, int port, const t_ipv6dot & peering_hip);
		t_peering_reference(const c_ip46_addr & peering_addr, const t_ipv6dot & peering_hip);

	public:
		c_ip46_addr peering_addr;
		c_haship_addr haship_addr;
};

// TODO: crypto options here
class c_peering { ///< An (mostly established) connection to peer
	public:
		c_peering(const t_peering_reference & ref);

		virtual void send_data(const char * data, size_t data_size);
		virtual ~c_peering()=default;

		virtual void print(ostream & ostr) const;

		virtual c_haship_addr get_hip() const;
		virtual c_haship_pubkey * get_pub() const; ///< gets "reference" to current pubkey; will be invlidated, use immediatelly
		virtual c_ip46_addr get_pip() const;

		virtual void set_pubkey( std::unique_ptr<c_haship_pubkey> && pubkey ); ///< consume this pubkey and set as mine
		bool is_pubkey() const; ///< do we have a valid pubkey set
		void add_limit_points(long int points);
		void decrement_limit_points();
		long int get_limit_points();

		friend class c_tunserver;

	protected:
		c_ip46_addr	m_peering_addr; ///< peer physical address in socket format
		c_haship_addr m_haship_addr; ///< peer haship address
		unique_ptr<c_haship_pubkey> m_pubkey; ///< his pubkey (when we know it)
		std::atomic<long int> m_limit_points; // decrement when send packet to this peer
};

ostream & operator<<(ostream & ostr, const c_peering & obj);

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
		#ifdef __linux__
		c_peering_udp(const t_peering_reference & ref, c_udp_wrapper_linux &udp_wrapper);
		// __linux__
		#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MACH__) // (multiplatform boost::asio)
		c_peering_udp(const t_peering_reference & ref, c_udp_wrapper_asio &udp_wrapper);
		#endif

		virtual void send_data(const char * data, size_t data_size) override;
		virtual void send_data_udp(const char * data, size_t data_size, int udp_socket,
			c_haship_addr src_hip, c_haship_addr dst_hip, int ttl, antinet_crypto::t_crypto_nonce nonce_used);
		virtual void send_data_udp_cmd(c_protocol::t_proto_cmd cmd, const string_as_bin & bin, int udp_socket);
	private:

		virtual void send_data_RAW_udp(const char * data, size_t data_size, int udp_socket); ///< direct write
		#ifdef __linux__
		std::reference_wrapper<c_udp_wrapper_linux> m_udp_wrapper; // TODO: sahred_ptr ?
		// __linux__
		#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MACH__) // (multiplatform boost::asio)
		std::reference_wrapper<c_udp_wrapper_asio> m_udp_wrapper; // TODO: sahred_ptr ?
		#endif
};

#endif // C_PEERING_H
