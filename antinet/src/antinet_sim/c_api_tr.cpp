#include "c_api_tr.hpp"
#include<sstream>



template<class Archive>
void t_hw_message::serialize(Archive & ar, const unsigned int version){			//all struct t_hw_message should be in separate file - to avoid recompilation of template
	ar & m_msg.m_data;
	ar & m_msg.m_remote_id;
	ar & m_type;
	/*
	 *	example use:
	 *
	 *
		t_hw_message mymsessage;
		ofstream file("file.txt");
		boost::archive::binary_oarchive oa(file);
		oa<<mymessage;


		ifstream file2("file.txt");
		boost::archive::binary_iarchive ia(file2);
		t_hw_message mm;
		ia>>mm;
	 *
	 *
	 */
}


void c_api_tr::write_message(t_message &&msg) {
	t_hw_message m_hw_message;
	m_hw_message.m_type = 0;
	m_hw_message.m_msg = std::move(msg);

	std::stringstream str;
	boost::archive::binary_oarchive oa(str);
	oa<<m_hw_message;

	hw_send(std::move(str.str()));
}

void c_api_tr::hw_recived(std::string &&serialized_msg) {

	m_incomming_msgs.push(std::move(serialized_msg));

}

void c_api_tr::read_message(std::function<void (t_message &&)> handler) {
	while(!m_incomming_msgs.empty()){
		std::string serialized_msg = m_incomming_msgs.front();
		m_incomming_msgs.pop();

		std::stringstream str;
		str.str(serialized_msg);		//check it!!
		boost::archive::binary_iarchive oia(str);
		t_hw_message m_hw_message;
		oia>>m_hw_message;
		//if(m_hw_message.type ==  sometype)
		handler(std::move(m_hw_message.m_msg));
		//
	}

	/*	t_message msg;
	handler (std::move(msg));
*/
}
