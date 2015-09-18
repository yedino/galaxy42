#include "c_api_tr.hpp"
#include<sstream>



template<class Archive>
void t_message::serialize(Archive & ar, const unsigned int version){			//all struct t_hw_message should be in separate file - to avoid recompilation of template
	ar & m_data;
	ar & m_remote_id;
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
	try {
	t_message m_message;
	m_message = std::move(msg);

	std::stringstream str;
	boost::archive::binary_oarchive oa(str);
	oa<<m_message;

	//hw_send(m_message.m_remote_id ,std::move(str.str()));
	} catch(...) {
		std::cout<<"c_api_tr::hw_recived in boost deserialization exeption"<<std::endl;

	}
}
/*
void c_api_tr::hw_recived(std::string &&serialized_msg) {

	std::string tmp_string (serialized_msg);
	try{
		std::stringstream str;
		str.str(tmp_string);		//check it!!
		boost::archive::binary_iarchive oia(str);
		t_message m_message;
		oia>>m_message;

		std::cout <<std::string ("c_api_tr::hw_recived")<<m_message.m_remote_id<<" " <<m_message.m_data;
	}catch(...){
		std::cout<<"c_api_tr::hw_recived in boost deserialization exeption"<<std::endl;
	}

	m_incomming_msgs.push(tmp_string);

}
*/
/*
void c_api_tr::read_message(std::function<void (t_message &&)> handler) {
	while(!m_incomming_msgs.empty()){
		std::string serialized_msg = m_incomming_msgs.front();
		m_incomming_msgs.pop();

		std::stringstream str;
		str.str(serialized_msg);		//check it!!
		boost::archive::binary_iarchive oia(str);
		t_message m_message;
		oia>>m_message;
		//if(m_hw_message.type ==  sometype)
		handler(std::move(m_message));
		//
	}

//		t_message msg;
//	handler (std::move(msg));


}*/