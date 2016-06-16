#include "c_json_genconf.hpp"



void c_json_genconf::genconf() {
	generate_galaxy_conf();
	generate_authpass_conf("connect_from.my.conf");
	generate_connectto_conf("connect_to.my.conf");
	generate_connectto_conf("connect_to.seed.conf");
}

void c_json_genconf::generate_galaxy_conf(const std::string &filename) {
	std::ofstream conf_file (filename, std::ios::out | std::ios::binary);
	if(conf_file.is_open()) {
		std::cout << "Generating main configuration file: " << filename << std::endl;

		conf_file << "{\n";
		conf_file << "\t// Your private key - the private key should be at bottom of this file.\n";
		conf_file << "\n";
		conf_file << "\t// Your public key - this key corresponds to the private key and ipv6 address:\n";
		conf_file << "\t\"myself-public\" : {\n";
		conf_file << "\t\t\"publicKey\": \"ed25519:1111111112222222233333333344444444445555555666666666.k\",\n";
		conf_file << "\t\t\"ipv6\": \"fd42:aaaa:bbbb:cccc:aaaa:bbbb:cccc:dddd\"\n";
		conf_file << "\t},\n";
		conf_file << "\t// *** ADD(1) *** add here peers that will connect to you (and/or instead add ones to which you will connect)\n";
		conf_file << "\t// 1) Here you create passwords for other nodes - anyone connecting and offering these passwords on connection will be allowed.\n";
		conf_file << "\t// We strongly recommend passwords to be random string of at least 40 characters; Or for easier typing: 8 not-obvious words randomly from dictionary plus some numbers.\n";
		conf_file << "\t// 2) Then you give your friend his password plus example data as seen below\n";
		conf_file << "\n";
		conf_file << "\t\"authorizedPasswords\" : [\n";
		conf_file << "\t\t\"connect_from.my.conf\"\t// <--- do not modify this line. Thanks to it, you can insert the new passwords in this file.\n";
		conf_file << "\t],\n";
		conf_file << "\n";
		conf_file << "\t/* EXAMPLE DATA FOR FRIEND:\n";
		conf_file << "--- Give your friend this text, after filling in the fields marked with XXX ----\n";
		conf_file << "\t\"XXX_HERE_WRITE_YOUR_OWN_IP_ADDRESS_OR_DOMAIN:9042\": {\n";
		conf_file << "\t\t\"password\": \"XXX_HERE_WRITE_ONE_OF_THE_PASSWORDS_ABOVE\",\n";
		conf_file << "\t\t\"publicKey\": \"ed25519:1111111112222222233333333344444444445555555666666666.k\",\n";
		conf_file << "\t\t\"ipv6\": \"fd42:aaaa:bbbb:cccc:aaaa:bbbb:cccc:dddd\",\n";
		conf_file << "\t}\n";
		conf_file << "--- above is for friend ---\n";
		conf_file << "\t*/\n";
		conf_file << "\n";
		conf_file << "\t\"connectTo\" : [\n";
		conf_file << "\t// --- below --- insert the credentials from friends\n";
		conf_file << "\t\t\"connect_to.my.conf\", // <--- do not modify this line. Thanks to it, you can insert the friend references also to file \"peers.conf\" as written here.\n";
		conf_file << "\t\t\"connect_to.seed.conf\" // <--- do not modify this line. It allows to use the default peers. You can remove this line or comment it out if you want.\n";
		conf_file << "\t// --- above --- insert the credentials from friends\n";
		conf_file << "\t],\n";
		conf_file << "\n";
		conf_file << "\t// Private key. Your confidentiality and data integrity depend on this key, keep it SECRET!!!\n";
		conf_file << "\t\"privateKeyType\": \"master-dh\",\n";
		conf_file << "\t\"privateKey\": \"ed25519:1fffffffffffffffffffaaaaaaaaaaabbbbbbbbbbbbbbbbcccccccccccc11111\"\n";
		conf_file << "}\n";
		conf_file.close();
	} else {
		throw std::invalid_argument("Fail to open file for write: " + filename);
	}
}

void c_json_genconf::generate_authpass_conf(const std::string &filename) {
	std::ofstream conf_file (filename, std::ios::out | std::ios::binary);
	if(conf_file.is_open()) {
		std::cout << "Generating authorized passwords configuration file: " << filename << std::endl;

		conf_file << "{\n";
		conf_file << "\t\"authorizedPasswords\" : [\n";
		conf_file << "\t\t{\"password\": \"C3yhZ8PjPoVFYwHfw0oKtNLxgMo5V9YUr7r4UXul\", \"myname\":\"default_public_password\"}\n";
		conf_file << "\t\t//{\"password\": \"Lq6yi0Ql9lcNWSDnCPophU7R0JWgyKwY5vrVEUrF\", \"myname\":\"password_nr1\"},\n";
		conf_file << "\t\t//{\"password\": \"S417y9vFrzbm6JyxTIotylPcsqyNFMufRL6BjwrA\", \"myname\":\"password_nr2\"},\n";
		conf_file << "\t\t//{\"password\": \"rpWG35w2JAjAizvRWeydOz739Hd11xgvvDufOAgn\", \"myname\":\"password_nr3\"}\n";
		conf_file << "\t\t// you can add more passwords here, or change the one above, e.g. change the \"myname\" field\n";
		conf_file << "\t\t// another example: {\"password\": \"uJUjh0wW9DW6FNri9pTVu44AsTuj9hwkfvhrBJ2z\", \"myname\":\"my_friend_from_the_bar\"}\n";
		conf_file << "\t]\n";
		conf_file << "}\n";
		conf_file.close();
	} else {
		throw std::invalid_argument("Fail to open file for write: " + filename);
	}
}

void c_json_genconf::generate_connectto_conf(const std::string &filename) {
	std::ofstream conf_file (filename, std::ios::out | std::ios::binary);
	if(conf_file.is_open()) {
		std::cout << "Generating peer references -- connnet to configuration file: " << filename << std::endl;

		conf_file << "{\n";
		conf_file << "\t\"connectTo\" : {\n";
		conf_file << "\t// --- below --- insert the credentials from friends\n";
		conf_file << "\n";
		conf_file << "\t\t/**\n";
		conf_file << "\t\t * Testing peers:\n";
		conf_file << "\t\t * // Peer nr 0\n";
		conf_file << "\t\t *  \"192.168.0.57\" : {\n";
		conf_file << "\t\t *  \t\"publicKey\" : \"asdasd\"\n";
		conf_file << "\t\t *  },\n";
		conf_file << "\t\t * // Peer nr 1\n";
		conf_file << "\t\t *  \"[fc72:aa65:c5c2:4a2d:54e:7947:b671:e00c]\" : {\n";
		conf_file << "\t\t *  \t\"publicKey\" : \"asdasd\"\n";
		conf_file << "\t\t *  }\n";
		conf_file << "\t\t */\n";
		conf_file << "\n";
		conf_file << "\t// --- above --- insert the credentials from friends\n";
		conf_file << "\t}\n";
		conf_file << "}\n";
		conf_file.close();
	} else {
		throw std::invalid_argument("Fail to open file for write: " + filename);
	}
}
