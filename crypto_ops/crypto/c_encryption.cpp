#include "c_encryption.hpp"

c_RSA::c_RSA () : m_crypto(new c_crypto_RSA<key_size>()) {
	m_crypto_method = RSA;
	// Generate RSA key
	m_crypto->generate_key();
	m_pub_key = m_crypto->get_public_key();
}

std::string c_RSA::get_public_key () {
	std::string n = m_pub_key.n.str();
	std::string e = m_pub_key.e.str();
	return e + '|' + n;
}

std::string c_RSA::sign (const std::string &msg) {
	m_last_sign = m_crypto->sign(msg);
	return m_last_sign.str();
}

bool c_RSA::verify(const std::string &signature, const std::string &message, const std::string &public_key) {
	rsa_public_key<long_type> pb(public_key);
	return (power_modulo(long_type(signature), pb.e, pb.n) == sha512<long_type>(message));
}

//////////////////////////////////////////////////////////  ED  ///////////////////////////////////////////////////////////////
c_ed25519::c_ed25519 () {
	m_crypto_method = ed25519;
	create_seed();
	create_keypair();
}

int c_ed25519::create_seed () {
	return ed25519_create_seed(m_seed);
}

void c_ed25519::create_keypair () {
	ed25519_create_keypair(m_public_key, m_private_key, m_seed);
}

////////////////////////////////////////////////////////// C WITH UNIQUE PTR ///////////////////////////////////////////////////

unique_ptr<unsigned char[]> c_ed25519::get_public_key_uC () {
	unique_ptr<unsigned char[]> pub_key (new unsigned char[pub_key_size+1]);
	for (int i = 0; i < pub_key_size; ++i) {
		pub_key[i] = m_public_key[i];
	}
	pub_key[pub_key_size] = '\0';
	return pub_key;
}

unique_ptr<unsigned char[]> c_ed25519::sign_uC (unique_ptr<const unsigned char[]> message, size_t message_len) {
	ed25519_sign(m_signature, message.get(), message_len, m_public_key, m_private_key);
	unique_ptr <unsigned char[]> sign (new unsigned char[sign_size+1]);
	for (int i = 0; i < sign_size; ++i) {
		sign[i] = m_signature[i];
	}
	sign[sign_size] = '\0';
	return sign;
}

bool c_ed25519::verify_uC (unique_ptr<const unsigned char[]> &signature,
						   unique_ptr<const unsigned char[]> &message,
						   size_t message_len,
						   unique_ptr<const unsigned char[]> &public_key) {

	return ed25519_verify(signature.get(), message.get(), message_len, public_key.get()) != 0;
}

////////////////////////////////////////////////////////// CPP INTERFACE ///////////////////////////////////////////////////////
std::string c_ed25519::get_public_key () {
	std::string pubkey = uchar_toReadable(m_public_key, pub_key_size);
	return pubkey;
}

string c_ed25519::sign (const string &msg) {
	unique_ptr<unsigned char []> umsg (string_to_uniqueUtab(msg));

	unique_ptr<unsigned char[]> sign_ustr = sign_uC(std::move(umsg), msg.length());
	std::string sign_str = uchar_toReadable(sign_ustr.get(), sign_size);

	return sign_str;
}

bool c_ed25519::verify (const std::string &signature,
						const std::string &message,
						const std::string &public_key) {
	size_t message_len = message.length();
	unique_ptr<const unsigned char[]> sign_u (readable_toUchar(signature, sign_size));
	unique_ptr<const unsigned char[]> msg_u (string_to_uniqueUtab(message));
	unique_ptr<const unsigned char[]> pubkey_u (readable_toUchar(public_key, pub_key_size));

	bool is_valid = verify_uC(sign_u,
							  msg_u,
							  message_len,
							  pubkey_u );
	return is_valid;
}
////////////////////////////////////////////////////////// C INTERFACE /////////////////////////////////////////////////////////
unsigned char* c_ed25519::get_public_key_C() {

	unsigned char* pub_key = new unsigned char[pub_key_size];
	for(int i = 0; i < pub_key_size; ++i) {
		pub_key[i] = m_public_key[i];
	}
	return pub_key;
}

unsigned char* c_ed25519::sign_C (const unsigned char *message, size_t message_len) {

	ed25519_sign(m_signature, message, message_len, m_public_key, m_private_key);
	unsigned char* sign = new unsigned char[sign_size];
	for(int i = 0; i < sign_size; ++i) {
		sign[i] = m_signature[i];
	}
	return sign;
}

int c_ed25519::verify_C(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key) {

	return ed25519_verify(signature, message, message_len, public_key);
}

/*
void c_ed25519::add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar) {

	ed25519_add_scalar(public_key, private_key, scalar);
}

void c_ed25519::key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key) {

	ed25519_key_exchange(shared_secret, public_key, private_key);
}*/

/////////////////////////////////////////////////////////////////////////////SUPPORT_FUNCTIONS////////////////////////////////////////////////////////////////////////////////////

void parse_crypto_data (const char *data, unsigned char *crypto_input) {
	const size_t bufsize = 2;

	for (size_t i = 1; (data + i) != nullptr && data[i * 2] != ':'; ++i) {
		char buffer[bufsize + 1];
		std::strncpy(buffer, data + i * 2, bufsize);
		buffer[bufsize] = '\0';

		int num = std::stoi(std::string(buffer), nullptr, 16);
		crypto_input[i-1] = (unsigned char)num;
	}
}

unique_ptr<unsigned char[]> string_to_uniqueUtab(const std::string &str) {
	size_t str_length = str.length();
	unique_ptr<unsigned char []> umsg (new unsigned char[str_length+1]);
	for(size_t i = 0; i < str_length; ++i) {
		umsg[i] = static_cast<unsigned char>(str.at(i));
	}
	umsg[str_length] = '\0';

	return umsg;
}

// constructing ed string format : ed23ff05.. where all 2 bytes after "ed" is one byte in original C unsigned char tab
std::string uchar_toReadable(const unsigned char* utab, ed25519_sizes size) {
	size_t length = 0;
	std::stringstream ss;
	ss << "ed";
	for(size_t i = 0; i < size; ++i) {
		static_cast<int>(utab[i]) > 15 ? 	// need for keep constant length
		ss << std::hex << static_cast<int>(utab[i]) 	:
		   ss << "0" << std::hex << static_cast<int>(utab[i]);	// 4 == 04
		length++;
	}
	std::string str = ss.str();
	return str;
}

unique_ptr<unsigned char[]> readable_toUchar(const std::string &str, ed25519_sizes size) {
	unique_ptr<unsigned char[]> utab (new unsigned char[size+1]);
	utab[size] = '\0';
	const size_t bufsize = 2;
	if(size == ((str.size()-2)/2)) {		// -2 because 2 first bytes is "ed"
		//std::cout << "sizes ok" << std::endl;	// dbg
	} else {
		std::cout << "ed: error! bad string in readable_toUchar" << std::endl;
		return nullptr;
	}
	for(size_t i = 0; i < size; ++i) {
		char buffer[bufsize+1];
		if(str.copy(buffer, bufsize, i*2+2) != bufsize) {
			std::cout << "ed: str.copy error" << std::endl;
		}
		buffer[bufsize] = '\0';
		int num = std::stoi(buffer,nullptr,16);
		utab[i] = num;
	}
	return utab;
}
