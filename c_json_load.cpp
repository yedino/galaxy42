#include "c_json_load.hpp"

c_json_file_parser::c_json_file_parser (const std::string &filename) {

    if(parse_file(filename)) {
        std::string msg = filename + " : bad configuration file";
        throw std::invalid_argument(msg);
    }
}

Json::Value c_json_file_parser::get_root() {
	return m_root;
}

bool c_json_file_parser::parse_file(const std::string &filename) {

	if(!filestorage::is_file_ok(filename)) {
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

c_auth_password_load::c_auth_password_load(const std::string &filename, std::vector<t_auth_password> &auth_passwords) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();

		get_auth_passwords(auth_passwords); ///< proper auth loading
	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename << " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

void c_auth_password_load::get_auth_passwords (std::vector<t_auth_password> &auth_passwords) {

	Json::Value authpass_array = m_root["authorizedPasswords"];
	size_t i = 0;
	for(auto &authpass : authpass_array) {
		std::string l_pass = authpass["password"].asString();
		std::string l_myname = authpass["myname"].asString();
		std::cout 	<< "auth ["<< i << "] : " << l_pass							//dbg
					<< " with public key [" << l_myname << ']' <<  std::endl;		//dbg
		auth_passwords.push_back({l_pass, l_myname});
		i++;
	}
}

c_connect_to_load::c_connect_to_load(const std::string &filename, std::vector<t_peering_reference> &peer_refs) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();

		get_peers(peer_refs);	///< proper peer loading
	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename <<  " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

void c_connect_to_load::get_peers(std::vector<t_peering_reference> &peer_refs) {

	Json::Value connectTo_array = m_root["connectTo"];
	uint32_t i = 0;
	for(auto &peer_ref : connectTo_array) {
		std::string l_ip = connectTo_array.getMemberNames()[i];
		std::string l_hip = peer_ref["publicKey"].asString();
		std::cout 	<< "Peer ["<< i << "] : " << l_ip							//dbg
					<< " with HIP [" << l_hip << ']' <<  std::endl;		//dbg

		// try to parse the :port
		size_t pos = l_ip.find(':');
		if(pos != std::string::npos) { // if there is :port
			auto pair = tunserver_utils::parse_ip_string(l_ip);
			peer_refs.emplace_back(t_peering_reference(pair.first, pair.second, l_hip));
		} else {
			peer_refs.emplace_back(t_peering_reference(l_ip, l_hip));
		}
		i++;
	}
}

c_galaxyconf_load::c_galaxyconf_load(const std::string &filename) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();
		t_my_keypair my_keypair = my_keypair_load();
		std::cout << "my info:\nprivKeyType[" << my_keypair.m_private_key_type
				  << "]\nprivKey[" << my_keypair.m_private_key
				  << "]\npubKey[" << my_keypair.m_public_key
				  << "]\nipv6[" << my_keypair.m_ipv6
				  << "]" << std::endl;

		auth_password_load();
		connect_to_load();

	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename << " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

std::vector<t_peering_reference> c_galaxyconf_load::get_peer_references() {
	return m_peer_references;
}

t_my_keypair c_galaxyconf_load::my_keypair_load() {
	std::string private_key_type = m_root.get("privateKeyType","").asString();
	if(private_key_type == "") {
		throw std::invalid_argument("empty privateKeyType field in your configuration file");
	}
	std::string private_key = m_root.get("privateKey","").asString();
	if(private_key == "") {
		throw std::invalid_argument("empty privateKey field in your configuration file");
	}
	std::string public_key = m_root.get("myself-public","").get("publicKey","").asString();
	if(public_key == "") {
		throw std::invalid_argument("empty publicKey field in your configuration file");
	}
	std::string ipv6 = m_root.get("myself-public","").get("ipv6","").asString();
	if(private_key_type == "" || private_key == "" || public_key == "" || ipv6 == "") {
		throw std::invalid_argument("empty ipv6 field in your configuration file");
	}
	return t_my_keypair({private_key_type,private_key,public_key,ipv6});
}

void c_galaxyconf_load::auth_password_load() {
	if(m_root.get("authorizedPasswords","").isArray()) {
		Json::Value authpass_array = m_root.get("authorizedPasswords","");
		for(auto &filename : authpass_array) {
			std::cout << "Loading authorizedPassword file: " << filename.asString() << std::endl;
			c_auth_password_load authpass_load(filename.asString(), m_auth_passwords);
			for(auto &authpass : m_auth_passwords) {
				std::cout << "auth -- password[" << authpass.m_password << "] myname["
						  << authpass.m_myname << "]" << std::endl;
			}
		}
	} else {
		throw std::invalid_argument("Empty authorizedPasswords in " + m_filename + " file");
	}
}

void c_galaxyconf_load::connect_to_load() {
	if(m_root.get("authorizedPasswords","").isArray()) {
		Json::Value connect_to = m_root.get("connectTo","");
		for(auto &filename : connect_to) {
			std::cout << "Loading connectTo file: " << filename.asString() << std::endl;
			c_connect_to_load connect_to_load(filename.asString(), m_peer_references);
			for(auto &peer : m_peer_references) {
				std::cout << "peer -- ip[" << peer.peering_addr << "]"
				<< " hip[" << peer.haship_addr << "]" << std::endl;
				// we do not load here pubkeys, just hip
			}
		}
	} else {
		throw std::invalid_argument("Empty connectTo in " + m_filename + " file");
	}

}

// test - simple usage
int json_test() {

  try {
	c_json_genconf::genconf();
	c_galaxyconf_load galaxyconf("galaxy.conf");

  } catch (std::exception &err) {
		std::cout << err.what() << std::endl;
		return 1;
  }

	return 0;
}
