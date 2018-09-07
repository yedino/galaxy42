#include <iostream>
#include <utility>
#include <boost/asio.hpp>

namespace detail {
    template <std::size_t N>
    struct msgs {
        const std::size_t size = N;
        std::array<mmsghdr, N> hdr;
        std::array<sockaddr_in, N> addr;
        std::array<iovec, N> msg;
        std::array<std::vector<unsigned char>, N> msg_data;
	};
}

class cSendmmsg_udp
{
public:
	cSendmmsg_udp(int socket);
	/** Default destructor */
	virtual ~cSendmmsg_udp();

	virtual size_t send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr);
	virtual size_t recv(unsigned char * data, size_t data_size, const boost::asio::ip::address & adr, boost::asio::ip::address & adr_out);
private:
	int m_socket;
    constexpr static unsigned int m_q_max_len = 10;
	unsigned int m_q_len = 0;
    detail::msgs<m_q_max_len> msgs_q;
	const unsigned short m_port = 9876;
};

cSendmmsg_udp::cSendmmsg_udp(int socket)
:	m_socket(socket)
{
	
}

cSendmmsg_udp::~cSendmmsg_udp() {
	close(m_socket);
}

size_t cSendmmsg_udp::send(const unsigned char * data, size_t data_size, const boost::asio::ip::address & adr) {
	
    struct sockaddr_in & my_addr = msgs_q.addr[m_q_len];
    iovec & msgvec = msgs_q.msg[m_q_len];
    msghdr & msg = msgs_q.hdr[m_q_len].msg_hdr;
    std::vector<unsigned char> & msg_data = msgs_q.msg_data[m_q_len];
	msg_data.insert( msg_data.begin(), data, data + data_size );
	
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_port = htons( m_port ); // short, network byte order
    my_addr.sin_addr.s_addr = *((int*)(adr.to_v4().to_bytes().data()));//inet_addr( "192.168.1.105" );//INADDR_ANY;
    memset( &( my_addr.sin_zero ), '\0', 8 );
	
	msgvec.iov_base = msg_data.data();
	msgvec.iov_len = msg_data.size();
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = &my_addr;
	msg.msg_namelen = sizeof(my_addr);
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;
	msg.msg_iov = &msgvec;
	msg.msg_iovlen = 1;

	m_q_len++;
	if( m_q_len == m_q_max_len ) {
		size_t bytes_sended = 0;
        int ret = sendmmsg(m_socket, msgs_q.hdr.data(), m_q_len, 0);
        for(unsigned int i = 0; i < m_q_len; i++ ) {
            bytes_sended += msgs_q.hdr[i].msg_len;
            msgs_q.msg_data[i].clear();
        }
        m_q_len = 0;
        return bytes_sended;
	}
	
	return 0;
}

size_t cSendmmsg_udp::recv(unsigned char * data, size_t data_size, const boost::asio::ip::address & adr, boost::asio::ip::address & adr_out) {
    throw std::runtime_error("not implemented");
	return 0;
}


int main(int argc, const char **argv) {
	std::cout << "multisend UDP. Usage: program dstip msgsize" << std::endl;
	constexpr int max_msgsize = 10000; // taken from max possible MTU we expect to see
	std::vector< std::string > args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	std::string dst_ip = args.at(1);
	int msgsize = atoi( args.at(2).c_str() );

	std::vector<unsigned char> buffer(msgsize, '0');
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	cSendmmsg_udp udp(sockfd);

	auto dst_ip_asio = boost::asio::ip::address::from_string(dst_ip);
	while(true) {
		udp.send(buffer.data(), buffer.size(), dst_ip_asio ); // TODO
	}
}
