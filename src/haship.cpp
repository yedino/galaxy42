// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt



#include "haship.hpp"
#include "strings_utils.hpp"

#include <boost/asio.hpp>
#include <sodium.h>


// c_haship_addr :
c_haship_addr::c_haship_addr()
	 : std::array<unsigned char, g_haship_addr_size>({{}})
{ }

c_haship_addr::c_haship_addr(tag_constr_by_hash_of_pubkey, const c_haship_pubkey & pubkey)
	 : std::array<unsigned char, g_haship_addr_size>({{}})
{
	_info("Creating HIP from pubkey: " << pubkey);

	string addr = pubkey.get_ipv6_string_bin();
	for (size_t i=0; i<16; ++i) at(i) = addr.at(i);
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_dot, const t_ipv6dot & addr_string) {
	_dbg1("******************PARSING IP: addr_string " << addr_string);
	// use boost asio for parsing
	boost::asio::ip::address_v6 asio_addr_v6;
	try {
		asio_addr_v6 = boost::asio::ip::address_v6::from_string(addr_string);
		boost::asio::ip::address_v6::bytes_type asio_addr_bytes = asio_addr_v6.to_bytes();

		assert (asio_addr_bytes.size() == 16); // 16 = 128/8
		for (int i = 0; i < 128 / 8; ++i) {
			this->at(i) = asio_addr_bytes.at(i);
		}
	} catch (boost::exception &err) {
		_throw_error(std::invalid_argument("The IP address looks invalid ["+addr_string+"]"));
	}
	_dbg1("Parsed string addr :" << asio_addr_v6.to_string());
	_dbg1("Parsed bytes addr  :" << *this);
}

c_haship_addr::c_haship_addr(tag_constr_by_addr_bin, const t_ipv6bin & data ) {
	if (! ( this->size() == data.size() ) ) {
		ostringstream oss; oss << "Trying to set hip address from binary data " << to_debug_b(data);
		_throw_error( std::runtime_error(oss.str()) );
	}
	for (size_t i=0; i<this->size(); ++i) this->at(i) = data.at(i);
}

void c_haship_addr::print(ostream &ostr) const {
	string_as_hex dbg( string_as_bin(*this) );
	ostr << "hip:" << dbg.get();
}

string c_haship_addr::get_hip_as_string(bool with_dots) const{
    string_as_hex dbg( string_as_bin(*this) );
    string hip(dbg.get());
    ostringstream out;
    for(auto it = hip.begin(); it < hip.end(); it++)
    {
        if(with_dots && it != hip.begin() && (it - hip.begin()) % 4 == 0)
            out << ':';
        out << *it;
    }
    return out.str();
}
ostream& operator<<(ostream &ostr, const c_haship_addr & v) {	v.print(ostr);	return ostr; }

void c_haship_pubkey::print(ostream &ostr) const {
	ostr << "pub:" << ::to_debug(this->serialize_bin());
}
ostream& operator<<(ostream &ostr, const c_haship_pubkey & v) {	v.print(ostr);	return ostr; }



// c_haship_pubkey :
c_haship_pubkey::c_haship_pubkey() { }

c_haship_pubkey::c_haship_pubkey( const string_as_bin & input ) {
	_mark("Loadig pubkey from: " << ::to_debug(input) );
	this->load_from_bin(input.bytes);
	_mark("After loading: " << (*this) );
	//for(size_t i=0; i<input.bytes.size(); ++i) at(i) = input.bytes.at(i);
//	for(auto v : input.bytes) at(
}

bool addr_is_galaxy(c_haship_addr addr) {
	if (addr.at(0) != 0xFD) return false;
	if (addr.at(1) != 0x42) return false;
	return true;
}
