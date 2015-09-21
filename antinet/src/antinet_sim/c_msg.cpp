#include "c_msg.hpp"

#define SERIALIZE std::stringstream str;\
	try{\
		boost::archive::text_oarchive oa(str);\
		oa<<*this;\
	}catch(...){\
	}\
	std::cout<<"output"<<str.str()<<std::endl; \
	return str.str()



msgcjd::msgcjd (const t_msgkind &logic) : m_logic(logic), m_ID(std::rand()) // TODO
{
}


msg_ping_request::msg_ping_request () : msgcjd (e_msgkind_ping_request) {
}


msg_dht_hello::msg_dht_hello():msgcjd(e_msgkind_dht_hello) {

}

msg_ping_response::msg_ping_response () : msgcjd (e_msgkind_ping_response) {
}

msg_buy::msg_buy (const t_msgkind &logic) : msgcjd(logic) {
}

msg_buy_inq::msg_buy_inq () : msg_buy(e_msgkind_buy_net_inq) {
}

msg_buy_menu::msg_buy_menu () : msg_buy(e_msgkind_buy_net_menu) {
}

msg_buy_buying::msg_buy_buying () : msg_buy(e_msgkind_buy_net_buying) {
}

msg_buy_agreed::msg_buy_agreed () : msg_buy(e_msgkind_buy_net_agreed) {
}

msg_buy_final::msg_buy_final () : msg_buy(e_msgkind_buy_net_final) {
}

msg_buy_currency_inq::msg_buy_currency_inq() : msg_buy(e_msgkind_buy_currency) {
}

msg_use::msg_use () : msgcjd(e_msgkind_data) {
}

msg_use_ftp::msg_use_ftp () {
}


template <class Archive >void msgcjd::serialize(Archive &ar, const unsigned int version){
	ar & m_ttl;
	ar & m_to;
	ar & m_from;
	ar & m_logic;
	ar & m_destination;
	ar &  m_ID;
}

template <class Archive >
void msg_buy_menu::serialize (Archive &ar, const unsigned int version) {
	ar & boost::serialization::base_object<msgcjd>(*this);
	ar & m_my_price;
}

template <class Archive >
void msg_use::serialize (Archive &ar, const unsigned int version) {
	ar & boost::serialization::base_object<msgcjd>(*this);
	ar & m_type;
	ar & m_data;
	ar & m_payment;
}

template <class Archive >
void msg_ping_response::serialize (Archive &ar, const unsigned int version) {
	ar & boost::serialization::base_object<msgcjd>(*this);
	ar & m_ping_time;
}



template<class Archive>
void msg_dht_hello::serialize(Archive &ar, const unsigned int version) {
	ar & boost::serialization::base_object<msgcjd>(*this);
	ar &  m_target_dht_address;
	ar & m_home_dht_address;
	ar & m_known_nodes;

}


template<class Archive>
void msg_ping_request::serialize(Archive &ar, const unsigned int version) {
	ar & boost::serialization::base_object<msgcjd>(*this);
}

namespace msg_serialization {

std::string serialize_message (msgcjd *message) {
	std::stringstream ss;
	boost::archive::text_oarchive oa{ss};
	oa << message;
	return ss.str();
}

std::shared_ptr<msgcjd> deserialize_message (std::string serialized_msg) {
	std::stringstream ss(serialized_msg);
	boost::archive::text_iarchive ia{ss};
	msgcjd *message;
	ia >> message;
	std::shared_ptr<msgcjd> ret(message);
	return ret;
}


}//namespace msg_serialization



BOOST_CLASS_EXPORT(msgcjd)
BOOST_CLASS_EXPORT(msg_buy_menu)
BOOST_CLASS_EXPORT(msg_use)
BOOST_CLASS_EXPORT(msg_ping_response)
BOOST_CLASS_EXPORT(msg_dht_hello)
BOOST_CLASS_EXPORT(msg_ping_request)

