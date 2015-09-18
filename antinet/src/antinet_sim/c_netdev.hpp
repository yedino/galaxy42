#ifndef C_NETDEV_H
#define C_NETDEV_H

#include "c_api_tr.hpp"
#include "c_msg.hpp"
#include "c_network.hpp"
#if defined USE_API_TR
class c_network;
struct c_msgtx;

class c_netdev
{
public:

    shared_ptr <c_network> m_network;
    c_netdev(){;}
//    unique_ptr<c_msgtx> send_message();
//    void receive_message(unique_ptr<c_msgtx> &&message);
	std::vector<t_message> m_outbox;
	std::vector<t_message> m_inbox;


	void hw_send (t_message); 		///< send to remote node - add message to outbox
	vector<t_message> hw_recived();		/// returns and clear input buffer -for synchronicus use
	void set_network(std::shared_ptr<c_network> network_ptr);

//	void tick();						//waiting for inbox


	virtual ~c_netdev () = default;

};
#endif // USE_API_TR
#endif // C_NETDEV_H
