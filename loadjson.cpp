#include "loadjson.hpp"



c_json_file_parser::c_json_file_parser (const std::string &filename) {

    if(parse_file(filename)) {
        std::string msg = filename + " : bad configuration file";
        throw std::invalid_argument(msg);
    }
}

bool c_json_file_parser::parse_file(const std::string &filename) {

    Json::CharReaderBuilder rbuilder;
    std::ifstream galaxy_config(filename, std::ifstream::binary);
    // Configure the Builder, then ...
    std::string errs;
    bool parsingSuccessful = Json::parseFromStream(rbuilder, galaxy_config, &root, &errs);
    if (!parsingSuccessful) {
        // report to the user the failure and their locations in the document.
        std::cout  << "Failed to parse configuration\n"
                   << errs;
        return 1;
    }
    return 0;
}

std::vector<c_peer> c_json_file_parser::get_peers() {

    std::vector<c_peer> peers;

    Json::Value UDPInterfaces = root["interfaces"]["UDPInterfaces"];
    Json::Value connectTo = UDPInterfaces["connectTo"];
    uint32_t i = 0;
    for(auto &peer : connectTo) {
        std::string l_ip = connectTo.getMemberNames()[i];
        std::string l_pubkey = peer["publicKey"].asString();

        //std::cout 	<< "Peer ["<< i << "] : " << l_ip							//dbg
        //            << " with public key [" << l_pubkey << ']' <<  std::endl;		//dbg

        peers.push_back({i, l_ip, l_pubkey});
        i++;
    }
    return peers;
}


c_auth_password_load::c_auth_password_load(const std::__cxx11::string &filename) {
	try {
		c_json_file_parser parser(filename);

	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

c_connect_to_load::c_connect_to_load(const std::__cxx11::string &filename) {
	try {
		c_json_file_parser parser(filename);

	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

c_galaxyconf_load::c_galaxyconf_load(const std::__cxx11::string &filename) : m_filename(filename) {
	try {
		c_json_file_parser parser(filename);
		root = parser.get_root();
		auth_password_load();
		connect_to_load();

	} catch (std::invalid_argument &err) {
		std::cout << "Fail to load galaxy configuration file" << std::endl;
		std::cout << err.what() << std::endl;
	}
}

void c_galaxyconf_load::auth_password_load() {
	if(root.get("authorizedPasswords","").isArray()) {
		Json::Value authpass_array = root.get("authorizedPasswords","");
		for(auto &filename : authpass_array) {
			std::cout << "Load authorizedPassword file: " << filename.asString() << std::endl;
			c_auth_password_load authpass_load(filename.asString());
		}
	} else {
		throw std::invalid_argument("Empty authorizedPasswords in conf file");
	}
}

void c_galaxyconf_load::connect_to_load() {
	if(root.get("authorizedPasswords","").isArray()) {
		Json::Value authpass_array = root.get("authorizedPasswords","");
		for(auto &filename : authpass_array) {
			std::cout << "Load connectTo file: " << filename.asString() << std::endl;
			c_connect_to_load connect_to_load(filename.asString());
		}
	} else {
		throw std::invalid_argument("Empty connectTo in conf file");
	}

}

// test - usage
int main() {

  try {
	// Working example of json
	c_json_file_parser config_file("galaxy.conf");

	for(auto peer : config_file.get_peers()) {
		std::cout 	<< "Peer ["<< peer.m_nr << "] : " << peer.m_ip
					<< " with public key [" << peer.m_public_key << ']' <<  std::endl;
	}
	// Not working
	c_galaxyconf_load galaxyconf("a.conf");

  } catch (std::exception &err) {
		std::cout << err.what() << std::endl;
  }

	return 0;
}
