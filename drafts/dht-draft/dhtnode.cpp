#include "dhtnode.h"
#include "nearestfunct.h"
#include <random>
#include <climits>

void dht_node::send_to_phisical_addr(phisical_addr addr,trace n_trace) {
	wr->send(addr,n_trace);
}


void dht_node::send_to_dht_node(dht_addr,trace n_trace) {					// wysyla po adresie hashowanym
	// zrobic gettery- setery?
	dht_addr ID_target = n_trace.direction ? n_trace.target_address : n_trace.home_address;
	dht_addr next_dht_address = get_nearest_node(ID_target);
	n_trace.next_dht_address = next_dht_address;

	n_trace.trace_to_next_dht.clear();
	n_trace.trace_to_next_dht = known_dht_addresses.at(next_dht_address);
	phisical_addr p_addres = n_trace.trace_to_next_dht.front();
	n_trace.trace_to_next_dht.pop_front();

	send_to_phisical_addr(p_addres,n_trace);
}

dht_addr dht_node::get_nearest_node(dht_addr search_address) {

	nearestFunct2 func(search_address);
	return (*std::min_element(known_dht_addresses.begin(),known_dht_addresses.end(),func)).first;

}// looking for nearest node (by hash)

dht_addr dht_node::get_farest_node(dht_addr address){
    nearestFunct2 func(address);
    return (*std::max_element(known_dht_addresses.begin(),known_dht_addresses.end(),func)).first;
}

void dht_node::start_tracking(dht_addr target){
    trace tr;
    tr.home_address = my_dht_address;
    tr.target_address = target;
    send_to_dht_node(target,tr);
}

void dht_node::recive(trace m_trace) {

    if(m_trace.home_to_target_path.size()>100 ||m_trace.target_to_home_path.size()>100){
        //ubij -ewentualnie poinformuj noda ze sciezka zbyt dluga
    }

    //obsluzyc sytuacje gdy nikt nie zna noda!!!
    cout<<"dht_node::reciva"<<" "<<my_dht_address <<" "<<my_phisical_address <<endl;
	if(m_trace.home_address == my_dht_address) {

		list<phisical_addr> tmp_list;
		m_trace.target_to_home_path.size() < m_trace.home_to_target_path.size() ?
		std::copy(m_trace.target_to_home_path.begin(),m_trace.target_to_home_path.end(),std::back_inserter(tmp_list)):
		std::copy(m_trace.home_to_target_path.begin(),m_trace.home_to_target_path.end(),std::back_inserter(tmp_list));
		known_dht_addresses.insert(pair<dht_addr,list<phisical_addr> >(m_trace.target_address,tmp_list));

        cout<<"dht_node::reciva"<<" "<<my_dht_address <<" "<<my_phisical_address<<"get tracing packet" <<endl;
        //rote found - adding shortes of home_to_target /target_to_home
	} else if(m_trace.target_address == my_dht_address) {
        m_trace.direction = false;
		send_to_dht_node(m_trace.home_address,m_trace);
		//someone calls - respond to home_address
	} else if(my_dht_address == m_trace.next_dht_address) {
		dht_addr looking_for_address = m_trace.direction? m_trace.target_address:m_trace.home_address;

        // if distance of  my_dht_address < looking_for_address my address is nearst - something goes wrong
            nearestFunct2 func(m_trace.home_address);
            if(func(my_dht_address,looking_for_address)){
                std::cout<<"went wrong"<<endl;
                return;


            }
		send_to_dht_node( looking_for_address,m_trace);
	} else {
        //go ahed by physical trace
        try{
            phisical_addr next_p_adress = m_trace.trace_to_next_dht.front();
            m_trace.trace_to_next_dht.pop_front();
            send_to_phisical_addr(next_p_adress,m_trace);
        }catch(...){
        }
	}

}// recieve tracking packet - checks - end_end_point/dht_endpoint/transit

void dht_node::send_to_phisical_addr(phisical_addr addr,nodes_path pth)
{
    wr->send(addr,pth);
}

void dht_node::recived_known_node_question(nodes_path pth){

    if(pth.home_address == my_dht_address) {
        list<phisical_addr> tmp_list;
        pth.target_to_home_path.size() < pth.home_to_target_path.size() ?
        std::copy(pth.target_to_home_path.begin(),pth.target_to_home_path.end(),std::back_inserter(tmp_list)):
        std::copy(pth.home_to_target_path.begin(),pth.home_to_target_path.end(),std::back_inserter(tmp_list));
		known_dht_addresses.insert(pair<dht_addr,list<phisical_addr> >(pth.target_address,tmp_list));
        //rote found - adding shortes of home_to_target /target_to_home
    } else if(pth.target_address == my_dht_address) {
        pth.direction = false;
        send_to_dht_node(pth.home_address,pth);
        //someone calls - respond to home_address
    } else if(my_dht_address == pth.next_dht_address) {
		dht_addr looking_for_address = pth.direction? pth.target_address:pth.home_address;
        send_to_dht_node( looking_for_address,pth);
    } else {
        //go ahed by physical trace
        phisical_addr next_p_adress = pth.trace_to_next_dht.front();
        pth.trace_to_next_dht.pop_front();
        send_to_phisical_addr(next_p_adress,pth);
    }
}

void dht_node::asked_for_known_node(nodes_path pth)
{
    list<phisical_addr> path_to_node;
    try{

       path_to_node = known_dht_addresses.at(pth.destination);

    }catch(...){

    }

//	std::copy(m_trace.target_to_home_path.begin(),m_trace.target_to_home_path.end(),std::back_inserter(tmp_list)):

}

void dht_node::recive_hello_pkg(hello_trace_packet pkg){

/*
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1, 6);

	for (int n=0; n<10; ++n)
		std::cout << dis(gen) << ' ';
	std::cout << '\n';
	*/

	if(pkg.direction){

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0,LONG_LONG_MAX);

		pkg.direction = false;
		pkg.home_dht_address = dis(gen);			//geting random dht address
		pkg.target_dht_addres = get_dht_addr();
		list<phisical_addr> tmp_node_list;
		tmp_node_list.push_back(my_phisical_address);

		std::pair<dht_addr , list <phisical_addr> >record;
		record.first = my_dht_address;
		record.second = tmp_node_list;
		pkg.known_nodes.insert(record);

		dht_addr nearest_node = get_nearest_node(pkg.home_dht_address);
		pkg.known_nodes.insert(std::pair <dht_addr , list <phisical_addr> >(nearest_node,known_dht_addresses.at(nearest_node)));
		dht_addr farest_node = get_farest_node(pkg.home_dht_address);
		pkg.known_nodes.insert(std::pair <dht_addr , list <phisical_addr> >(farest_node,known_dht_addresses.at(farest_node)));

		tmp_node_list.clear();
		tmp_node_list.push_back(pkg.home_address);
		known_dht_addresses.insert(std::pair <dht_addr , list <phisical_addr> >(pkg.home_dht_address , tmp_node_list));

	}else{

		my_dht_address = pkg.home_address;
		for (auto it : pkg.known_nodes){
			known_dht_addresses.insert(it);
		}
		// obsluz tablice dostarczona przez
	}
}

void dht_node::hello_ping()
{
	hello_trace_packet pkg;
	pkg.home_address = my_phisical_address;
//	pkg.
}
