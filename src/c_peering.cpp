// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "c_peering.hpp"

#include <sodium.h>

#include "protocol.hpp"

#include "trivialserialize.hpp"

// ------------------------------------------------------------------

t_peering_reference::t_peering_reference(const t_ipv46dot & peering_addr, int port, const t_ipv6dot & peering_hip)
	: t_peering_reference( c_ip46_addr(peering_addr, port) , peering_hip )
{ }

t_peering_reference::t_peering_reference(const c_ip46_addr & peering_addr, const t_ipv6dot & peering_hip)
	:
	peering_addr( peering_addr )
	,haship_addr( c_haship_addr::tag_constr_by_addr_dot() , peering_hip  )
{
	_info("peering REFERENCE created, now peering_addr=" << this->peering_addr << " on port="
		  << peering_addr.get_assign_port() << ", and this is: " << (*this) );
}

// ------------------------------------------------------------------

c_peering::c_peering(const t_peering_reference & ref)
	:
	m_peering_addr(ref.peering_addr)
	,m_haship_addr(ref.haship_addr)
	,m_pubkey(nullptr) // unknown untill we e.g. download it; was: make_unique<c_haship_pubkey>(ref.pubkey))
	,m_limit_points(100)
{ }

void c_peering::print(ostream & ostr) const {
	ostr << "peering{";
	ostr << " peering-addr=" << m_peering_addr;
	ostr << " hip=" << m_haship_addr;
	ostr << " pub=" << to_debug(m_pubkey);
	ostr << "}";
}

void c_peering::send_data(const char * data, size_t data_size) {
	UNUSED(data); UNUSED(data_size);
	throw std::runtime_error("Used abstract send_data() that does nothing");
}

ostream & operator<<(ostream & ostr, const c_peering & obj) {	obj.print(ostr); return ostr; }

c_haship_addr c_peering::get_hip() const { return m_haship_addr; }
c_haship_pubkey * c_peering::get_pub() const { return m_pubkey.get(); }
c_ip46_addr c_peering::get_pip() const { return m_peering_addr; }

void c_peering::set_pubkey( std::unique_ptr<c_haship_pubkey> && pubkey ) {
	m_pubkey = std::move(pubkey);
}

bool c_peering::is_pubkey() const { return m_pubkey != nullptr; }

void c_peering::add_limit_points(long points) {
	m_limit_points += points;
}

void c_peering::decrement_limit_points() {
	m_limit_points--;
}

long c_peering::get_limit_points() {
	return m_limit_points;
}

// ------------------------------------------------------------------

c_peering_udp::c_peering_udp(const t_peering_reference & ref, c_udp_wrapper_linux &udp_wrapper)
	: c_peering(ref)
	,m_udp_wrapper(udp_wrapper)
{ }


void c_peering_udp::send_data(const char * data, size_t data_size) {
	UNUSED(data); UNUSED(data_size);
	throw std::runtime_error("Use send_data_udp");
}


// TODO unify array types! string_as_bin , unique_ptr to new c-array, raw c-array in libproto etc

void c_peering_udp::send_data_udp(const char * data, size_t data_size, int udp_socket,
	c_haship_addr src_hip, c_haship_addr dst_hip, int ttl, antinet_crypto::t_crypto_nonce nonce_used) {
	_info("Send to peer (tunneled data) data: " << string_as_dbg(data,data_size).get() ); // TODO .get

	trivialserialize::generator gen(data_size + 50);
	gen.push_byte_u( c_protocol::current_version );
	gen.push_byte_u( c_protocol::e_proto_cmd_tunneled_data );
	gen.push_bytes_n( g_ipv6_rfc::length_of_addr , to_binary_string(src_hip) );
	gen.push_bytes_n( g_ipv6_rfc::length_of_addr , to_binary_string(dst_hip) );
	gen.push_byte_u( ttl );
	gen.push_bytes_n( crypto_box_NONCEBYTES , nonce_used.get().to_binary() ); // TODO avoid conversion/copy
	gen.push_varstring( std::string(data, data+data_size)  ); // TODO view_string

/*
	// TODONOW turn off this crypto (unless leave here for peer-to-peer auth only)
	static unsigned char generated_shared_key[crypto_generichash_BYTES] = {43, 124, 179, 100, 186, 41, 101, 94, 81, 131, 17,
					198, 11, 53, 71, 210, 232, 187, 135, 116, 6, 195, 175,
					233, 194, 218, 13, 180, 63, 64, 3, 11};

	static unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES] = {148, 231, 240, 47, 172, 96, 246, 79};
	static unsigned char additional_data[] = {1, 2, 3};
	static unsigned long long additional_data_len = 3; // TODO

	// TODO randomize this data XXX use real crypto data

	const int header_size = c_protocol::version_size + c_protocol::cmd_size + c_protocol::ttl_size ; //  [protocol] TODO pack this into protocol class --r

	// TODO allocate buffer outside
	std::unique_ptr<unsigned char[]> protomsg(new unsigned char[data_size + crypto_aead_chacha20poly1305_ABYTES + header_size ]); // plus the headers
	// long long? or u64? --r assert size  sizeof(long long  .... == ...

	// why? --r:
	assert(crypto_aead_chacha20poly1305_KEYBYTES <= crypto_generichash_BYTES);

	// write headers:
	protomsg.get()[0] = c_protocol::current_version;
	protomsg.get()[1] = c_protocol::e_proto_cmd_tunneled_data; // this are tunneled data
	assert( (ttl >= 0) && (ttl <= c_protocol::ttl_max_value_ever) );
	protomsg.get()[2] = ttl; // ...this is their route ttl

	unsigned char * ciphertext_buf = protomsg.get() + header_size; // just-pointer to part of protomsg where to write the message!
	unsigned long long ciphertext_buf_len = 0; // encryption will write here the resulting size
    crypto_aead_chacha20poly1305_encrypt(ciphertext_buf, &ciphertext_buf_len, reinterpret_cast<const unsigned char *>(data), data_size, additional_data,
                                         additional_data_len, NULL, nonce, generated_shared_key);
	unsigned long long protomsg_len = ciphertext_buf_len + header_size; // the output of crypto, plus the header in front

	// TODO asserts!!!
*/

	string protomsg = gen.str(); // TODO view_string
	this->send_data_RAW_udp(protomsg.c_str(), protomsg.size(), udp_socket);
}

void c_peering_udp::send_data_udp_cmd(c_protocol::t_proto_cmd cmd, const string_as_bin & bin, int udp_socket) {
	_info("Send to peer (COMMAND): command="<<static_cast<int>(cmd)<<" data: " << string_as_dbg(bin).get() ); // TODO .get
	string_as_bin raw;
    raw.bytes += c_protocol::current_version;
    raw.bytes += cmd;
	raw.bytes += bin.bytes;
	this->send_data_RAW_udp(raw.bytes.c_str(), raw.bytes.size(), udp_socket);
}

void c_peering_udp::send_data_RAW_udp(const char * data, size_t data_size, int udp_socket) {
	_info("UDP send to peer RAW. To IP: " << m_peering_addr <<
		", RAW-DATA: " << to_debug_b(std::string(data,data_size)) );

	switch (m_peering_addr.get_ip_type()) {
		case c_ip46_addr::t_tag::tag_ipv4 : {
			auto ip_x = m_peering_addr.get_ip4(); // ip of proper type, as local variable
			m_udp_wrapper.get().send_data(m_peering_addr, data, data_size);
		}
		break;
		case c_ip46_addr::t_tag::tag_ipv6 : {
			auto ip_x = m_peering_addr.get_ip6(); // ip of proper type, as local variable
			m_udp_wrapper.get().send_data(m_peering_addr, data, data_size);
		}
		break;
		default: {
			std::ostringstream oss; oss << m_peering_addr; // TODO
			throw std::runtime_error(string("Invalid IP type (when trying to send RAW udp): ") + oss.str());
		}
	}
}

