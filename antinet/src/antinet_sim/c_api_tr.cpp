#include "c_api_tr.hpp"



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


void c_api_tr::read_message(std::function<void (t_message &&)> handler)
{
	t_message msg;
	handler (std::move(msg));
}
