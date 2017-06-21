// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "c_json_load.hpp"

c_json_file_parser::c_json_file_parser (const std::string &filename) {

    if(parse_file(filename)) {
        std::string msg = filename + " : bad configuration file";
        _throw_error( std::invalid_argument(msg) );
    }
}

Json::Value c_json_file_parser::get_root() {
	return m_root;
}

bool c_json_file_parser::parse_file(const std::string &filename) {

	if(!datastore::is_file_ok(filename)) {
		return 1;
	}

    Json::CharReaderBuilder rbuilder;
	std::ifstream file_stream(filename, std::ifstream::binary);
    // Configure the Builder, then ...
    std::string errs;
	bool parsingSuccessful = Json::parseFromStream(rbuilder, file_stream, &m_root, &errs);
    if (!parsingSuccessful) {
        // report to the user the failure and their locations in the document.
		std::cout  << "Failed to parse\n"
                   << errs;
        return 1;
    }
	return 0;
}

std::stringstream remove_comments(std::istream &istr) {
	std::stringstream ss;
	for (std::string line; std::getline(istr, line); ) {
			// load only lines that are not comment
			size_t position;
			if((position = line.find("//")) != std::string::npos) {
				line.erase(position);
			}
			if(line.length() == 0) continue;
			//std::cout << line << std::endl;
			ss << line;
	}
	return ss;
}
json file_parse(const std::string &filename) {
		json j;
		// read a JSON file
		std::ifstream file(filename);
		auto ss = remove_comments(file);
		ss >> j;
		return j;
}

c_auth_password_load::c_auth_password_load(const std::string &filename, std::vector<t_auth_password> &auth_passwords) : m_filename(filename) {
	try {
		m_json = file_parse(filename);

		get_auth_passwords(auth_passwords); ///< proper auth loading
	} catch (std::invalid_argument &err) {
		_info("Fail to load " << m_filename << " configuration file.\n" << err.what());
		throw;
	}
}

void c_auth_password_load::get_auth_passwords (std::vector<t_auth_password> &auth_passwords) {

	if(m_json.at("authorizedPasswords").is_array()) {
		auto auth_arr = m_json.at("authorizedPasswords");

		size_t i = 0;
		for(auto &authpass : auth_arr) {
			std::string l_pass = authpass.at("password").get<std::string>();
			std::string l_myname = authpass.at("myname").get<std::string>();
			_dbg3("Loaded authorized password ["<< i << "] : " << l_pass << " with name [" << l_myname << ']');
			auth_passwords.push_back({l_pass, l_myname});
			i++;
		}
	} else {
		_throw_error( std::invalid_argument("Bad format of authorizedPasswords in " + m_filename + " file") );
	}

}

c_connect_to_load::c_connect_to_load(const std::string &filename, std::vector<t_peering_reference> &peer_refs) : m_filename(filename) {
	try {
		m_json = file_parse(filename);

		get_peers(peer_refs);	///< proper peer loading
	} catch (std::invalid_argument &err) {
		_warn("Fail to load " << m_filename <<  " configuration file." << err.what());
		throw;
	}
}

void c_connect_to_load::get_peers(std::vector<t_peering_reference> &peer_refs) {

	if(m_json.at("connectTo").is_object()) {
		auto connect_arr = m_json.at("connectTo");

		for (json::iterator it = connect_arr.begin(); it != connect_arr.end(); ++it) {
			std::string l_ip = it.key();
			std::string l_hip = it.value().at("publicKey").get<std::string>();

			_dbg3("Peer loaded : " << l_ip << " with HIP [" << l_hip << ']');

			// parse  :port
			size_t pos = l_ip.find(':');
			if(pos != std::string::npos) { // if there is :port
				auto pair = tunserver_utils::parse_ip_string(l_ip);
				auto t_perref = t_peering_reference(pair.first, pair.second, l_hip);
				peer_refs.emplace_back(t_perref);
			} else {
				auto t_perref = t_peering_reference(l_ip, l_hip);
				peer_refs.emplace_back(t_perref);
			}
		}

	} else {
		_throw_error( std::invalid_argument("Bad format of connectTo in " + m_filename + " file") );
	}

}

c_galaxyconf_load::c_galaxyconf_load(const std::string &filename) : m_filename(filename) {
	try {
		m_json = file_parse(filename);

		t_my_keypair my_keypair = my_keypair_load();
		_dbg4 ("my info:\nprivKeyType[" << my_keypair.m_private_key_type
		       << "]\nprivKey[" << my_keypair.m_private_key
		       << "]\npubKey[" << my_keypair.m_public_key
		       << "]\nipv6[" << my_keypair.m_ipv6 << "]");

		auth_password_load();
		connect_to_load();

	} catch (std::invalid_argument &err) {
		_warn("Fail to load " << m_filename <<  " configuration file.\n" << err.what());
		throw;
	}
}

std::vector<t_peering_reference> c_galaxyconf_load::get_peer_references() {
	return m_peer_references;
}

std::vector<t_auth_password> c_galaxyconf_load::get_auth_passwords() {
	return m_auth_passwords;
}

t_my_keypair c_galaxyconf_load::my_keypair_load() {

	std::string private_key_type = m_json.at("privateKeyType").get<std::string>();
	std::string private_key = m_json.at("privateKey").get<std::string>();

	json mypub_obj = m_json.at("myself-public");

	if( !mypub_obj.is_object() ) {
		_throw_error( std::invalid_argument("Bad format of myself-public section in confuguration file.") );
	}

	std::string public_key = mypub_obj.at("publicKey").get<std::string>();
	std::string ipv6 = mypub_obj.at("ipv6").get<std::string>();

	return t_my_keypair({private_key_type,private_key,public_key,ipv6});
}

void c_galaxyconf_load::auth_password_load() {

	if(m_json.at("authorizedPasswords").is_array()) {
		auto auth_arr = m_json.at("authorizedPasswords").get<std::vector<std::string>>();
		for(auto &filename : auth_arr) {
			_dbg2("Loading authorizedPassword file: " << filename);
			c_auth_password_load authpass_load(filename, m_auth_passwords);
			for(auto &authpass : m_auth_passwords) {
				_dbg3("Loaded: authorized password [" << authpass.m_password
				      << "] myname [" << authpass.m_myname << "]");
			}
		}
	} else {
		_throw_error( std::invalid_argument("Bad format of authorizedPasswords in " + m_filename + " file") );
	}
}

void c_galaxyconf_load::connect_to_load() {

	if(m_json.at("connectTo").is_array()) {
		auto connect_arr = m_json.at("connectTo").get<std::vector<std::string>>();
		for(auto &filename : connect_arr) {
			_dbg2("Loading connectTo file: " << filename);
			c_connect_to_load connect_to_load(filename, m_peer_references);
			for(auto &peer : m_peer_references) {
				_dbg3("Loaded peer ip [" << peer.peering_addr << "]"
				      << " hip[" << peer.haship_addr << "]");
				// we do not load here pubkeys, just hip
			}
		}
	} else {
		_throw_error( std::invalid_argument("Bad format of connectTo in " + m_filename + " file") );
	}
}
