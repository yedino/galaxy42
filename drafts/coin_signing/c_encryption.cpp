#include "c_encryption.hpp"

c_RSA::c_RSA () : m_crypto(new c_crypto_RSA<key_size>()) {
	m_crypto_method = RSA;
	// Generate RSA key
	m_crypto->generate_key();
}

std::string c_RSA::get_public_key () {
	auto pub_key = m_crypto->get_public_key();
	std::string n = pub_key.n.str();
	std::string e = pub_key.e.str();
	return e + '|' + n;
}

std::string c_RSA::sign (const std::string &msg) {
	return m_crypto->sign(msg).str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

unsigned char *c_ed25519::get_public_key_C () {

    unsigned char *pub_key = new unsigned char[pub_key_size+1];
	for (int i = 0; i < pub_key_size; ++i) {
		pub_key[i] = m_public_key[i];
    }
    pub_key[pub_key_size] = '\0';
	return pub_key;
}

unsigned char *c_ed25519::sign_C (const unsigned char *message, size_t message_len) {

	ed25519_sign(m_signature, message, message_len, m_public_key, m_private_key);
    unsigned char *sign = new unsigned char[sign_size+1];
	for (int i = 0; i < sign_size; ++i) {
		sign[i] = m_signature[i];
    }
    sign[sign_size] = '\0';
	return sign;
}

int c_ed25519::verify_C (const unsigned char *signature,
	const unsigned char *message,
	size_t message_len,
	const unsigned char *public_key) {

	return ed25519_verify(signature, message, message_len, public_key);
}

////////////////////////////////////////////////////////// CPP INTERFACE ///////////////////////////////////////////////////////
std::string c_ed25519::get_public_key () {

    std::string pubkey = uchar_toReadable(m_public_key, pub_key_size);
    return pubkey;
}

string c_ed25519::sign (const string &msg) {

	unsigned char *sign_ustr = this->sign_C(reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length());
    std::string sign_str = uchar_toReadable(sign_ustr, sign_size);
	delete[] sign_ustr;
	return sign_str;
}

int c_ed25519::verify (const std::string signature,
	const std::string message,
	size_t message_len,
	std::string public_key) {

    const unsigned char *sign_u = readable_toUchar(signature, sign_size);
    const unsigned char *pubkey_u = readable_toUchar(public_key, pub_key_size);
	
    bool is_valid = verify_C(sign_u,
                          reinterpret_cast<const unsigned char *>(message.c_str()),
                          message_len,
                          pubkey_u
                          );
    delete [] sign_u;
    delete [] pubkey_u;

    return is_valid;
}
/*
void c_ed25519::add_scalar(unsigned char *public_key, unsigned char *private_key, const unsigned char *scalar) {
	
	ed25519_add_scalar(public_key, private_key, scalar);
}

void c_ed25519::key_exchange(unsigned char *shared_secret, const unsigned char *public_key, const unsigned char *private_key) {
	
	ed25519_key_exchange(shared_secret, public_key, private_key);
}*/

/////////////////////////////////////////////////////////////////////////////SUPPORT_FUNCTIONS////////////////////////////////////////////////////////////////////////////////////

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
unsigned char* readable_toUchar(const std::string &str, ed25519_sizes size) {
    unsigned char *utab = new unsigned char[size+1];
    utab[size] = '\0';
    const size_t bufsize = 2;
    if(size == ((str.size()-2)/2)) {		// -2 because 2 first bytes is "ed"
        //std::cout << "sizes ok" << std::endl;	// dbg
    }
    else {
        std::cout << "ed: error! bad string in readable_toUchar" << std::endl;
        delete [] utab;
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
