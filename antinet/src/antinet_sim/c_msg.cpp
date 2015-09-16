#include "c_msg.hpp"

msgcjd::msgcjd (const t_msgkind &logic) : m_logic(logic), m_ID(std::rand()) // TODO
{
}

std::string msgcjd::serialize() {
	std::stringstream str;
	try{
		boost::archive::text_oarchive oa(str);
		oa<<*this;
	}catch(...){

	}
	return str.str();
}
void msgcjd::deserialize(std::string serialized_obj) {
	try{
	msgcjd tmp_msg;
	std::stringstream str(serialized_obj);
	boost::archive::text_iarchive ia(str);

	//ia>>tmp_msg;
	ia >> *this;
	}catch(...){

	}
}

msg_ping::msg_ping () : msgcjd (e_msgkind_ping_request) {
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


BOOST_CLASS_EXPORT(msgcjd)
BOOST_CLASS_EXPORT(msg_buy_menu)
BOOST_CLASS_EXPORT(msg_use)
BOOST_CLASS_EXPORT(msg_ping_response)