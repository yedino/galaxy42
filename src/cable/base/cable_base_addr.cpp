
#include "libs1.hpp"
#include "cable/base/cable_base_addr.hpp"
#include "cable/udp/cable_udp_addr.hpp"
#include "cable/shm/cable_shm_addr.hpp"

const boost::any & c_cable_base_addr::get_addrdata() const { return m_addrdata ; }

c_cable_base_addr::c_cable_base_addr(boost::any && addrdata)
	: m_addrdata(std::move(addrdata))
{ }

void c_cable_base_addr::init_addrdata(boost::any && addrdata) {
	swap(m_addrdata, addrdata);
}

void c_cable_base_addr::print(std::ostream & ostr) const {
	_UNUSED(ostr);
	_throw_error_runtime("Trying to print abstract cable address");
	// ostr << "[unknown-address-type]";
}

std::ostream & operator<<(std::ostream & ostr , c_cable_base_addr & obj) {
	obj.print(ostr);
	return ostr;
}

unique_ptr<c_cable_base_addr> c_cable_base_addr::cable_make_addr(const string & str) {
	try{
		size_t pos1 = str.find(':');
		if (pos1 == string::npos) { // try if is udp (format 192.166.218.58)
			return std::make_unique<c_cable_udp_addr>(join_string(str, ":", get_default_galaxy_port()));
		}
		else {
			_try_user(pos1>0); // string::substr is safe, but anyway test against user doing --peer "-foo"
			string part1 = str.substr(0,pos1);
			string part2 = str.substr(pos1+1);
			if (part1=="auto" || part1=="udp" || part1=="tcp") { //udp for those types of cables
				return std::make_unique<c_cable_udp_addr>(part2);
			}
			else if(part1=="shm"){
				auto iter = find_if(part2.begin(), part2.end(), [](char c) { return !isalnum(c); });
				if( iter == part2.end() )
					return std::make_unique<cable_shm_addr>(part2);
				else
					_throw_error( std::invalid_argument(mo_file_reader::gettext("L_bad_peer_format")) );
			}
			else // try if is udp (format 192.166.218.58:9042)
			{
				return std::make_unique<c_cable_udp_addr>(str);
			}
		}
	}
	catch (const std::exception &e) {
		_erro(mo_file_reader::gettext("L_failed_adding_peer_simple_reference") << e.what());
		_throw_error( std::invalid_argument(mo_file_reader::gettext("L_bad_peer_format")) );
	}

	unique_ptr<c_cable_base_addr> x;
	return x;
}

bool operator==(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2) {
	return obj1.is_same(obj2);
}

bool operator<(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2) {
	return obj1.compare(obj2) < 0;
}

int get_default_galaxy_port(){
	return 9042;
}
