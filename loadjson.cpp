#include "libs1.hpp"
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

// test - usage
int main(int argc, char **argv) {

  try {
    c_json_file_parser config_file("galaxy.conf");

    for(auto peer : config_file.get_peers()) {
        std::cout 	<< "Peer ["<< peer.m_nr << "] : " << peer.m_ip
                    << " with public key [" << peer.m_public_key << ']' <<  std::endl;
    }
  } catch (std::exception &err) {
        std::cout << err.what() << std::endl;
  }

    return 0;
}
