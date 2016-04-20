#include "loadjson.hpp"


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

c_auth_password_load::c_auth_password_load(const std::string &filename) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();
	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename << " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

std::vector<c_auth_password> c_auth_password_load::get_authpass() {
	std::vector<c_auth_password> auth_passwords;

	std::cout << "getting auth passwords from " << m_filename << std::endl;
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
	return auth_passwords;
}

c_connect_to_load::c_connect_to_load(const std::string &filename) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();
	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename <<  " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

std::vector<c_peer> c_connect_to_load::get_peers() {
	std::vector<c_peer> peers;

	std::cout << "getting peers from " << m_filename << std::endl;

	Json::Value connectTo_array = m_root["connectTo"];
	uint32_t i = 0;
	for(auto &peer : connectTo_array) {
		std::string l_ip = connectTo_array.getMemberNames()[i];
		std::string l_pubkey = peer["publicKey"].asString();
		std::cout 	<< "Peer ["<< i << "] : " << l_ip							//dbg
					<< " with public key [" << l_pubkey << ']' <<  std::endl;		//dbg
		peers.push_back({l_ip, l_pubkey});
		i++;
	}
	return peers;
}

c_galaxyconf_load::c_galaxyconf_load(const std::string &filename) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		m_root = parser.get_root();
		auth_password_load();
		connect_to_load();

	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load " << m_filename << " configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

void c_galaxyconf_load::auth_password_load() {
	if(m_root.get("authorizedPasswords","").isArray()) {
		Json::Value authpass_array = m_root.get("authorizedPasswords","");
		for(auto &filename : authpass_array) {
			std::cout << "Loading authorizedPassword file: " << filename.asString() << std::endl;
			c_auth_password_load authpass_load(filename.asString());
			authpass_load.get_authpass();
			for(auto &authpass : authpass_load.get_authpass()) {
				std::cout << "auth -- password[" << authpass.m_password << "] myname[" << authpass.m_myname << "]" << std::endl;
			}
		}
	} else {
		throw std::invalid_argument("Empty authorizedPasswords in " + m_filename + " file");
	}
}

void c_galaxyconf_load::connect_to_load() {
	if(m_root.get("authorizedPasswords","").isArray()) {
		Json::Value connectto = m_root.get("connectTo","");
		for(auto &filename : connectto) {
			std::cout << "Loading connectTo file: " << filename.asString() << std::endl;
			c_connect_to_load connect_to_load(filename.asString());
			for(auto &peer : connect_to_load.get_peers()) {
				std::cout << "peer -- ip[" << peer.m_ip << "] pubkey[" << peer.m_public_key << "]" << std::endl;
			}
		}
	} else {
		throw std::invalid_argument("Empty connectTo in " + m_filename + " file");
	}

}

// test - usage
int main() {

  try {
	// Working example of json
//	c_json_file_parser config_file("galaxy.conf");

//	for(auto peer : config_file.get_peers()) {
//		std::cout 	<< "Peer ["<< peer.m_nr << "] : " << peer.m_ip
//					<< " with public key [" << peer.m_public_key << ']' <<  std::endl;
//	}
	// Not working
	c_galaxyconf_load galaxyconf("a.conf");

  } catch (std::exception &err) {
		std::cout << err.what() << std::endl;
  }

	return 0;
}
