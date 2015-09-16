#include "dht_world.h"
#include "dhtnode.h"

dht_world::dht_world() {
    create_world(4);

}

ID_addr dht_world::calculate_hash(phisical_addr addr) {
	return addr;
}

void dht_world::create_world(size_t size) {


             list<phisical_addr> tmp_path;
    dht_node node1(1);
             node1.set_phisical_addr(1);
                    tmp_path.push_back(2);
                    tmp_path.push_back(3);
                     node1.add_dht_address(3,tmp_path);
                     node1.set_world(this);
                tmp_path.clear();

    dht_node node2(255);
             node2.set_phisical_addr(2);
                     node2.set_world(this);
                     tmp_path.push_back(1);
                     node2.add_dht_address(1,tmp_path);


    tmp_path.clear();
    dht_node node3(3);
                node3.set_phisical_addr(3);

                tmp_path.push_back(2);
                node3.add_dht_address(255,tmp_path);
                node3.set_world(this);

                tmp_path.clear();
                tmp_path.push_back(4);
                tmp_path.push_back(5);
                node3.add_dht_address(5,tmp_path);
                tmp_path.clear();
                tmp_path.push_back(2);
                tmp_path.push_back(1);
                node3.add_dht_address(1,tmp_path);
                tmp_path.clear();


    dht_node node4(4);
                node4.set_phisical_addr(4);
                node4.set_world(this);



   dht_node node5(5);
            node5.set_phisical_addr(5);

            tmp_path.push_back(4);
            tmp_path.push_back(3);
            tmp_path.push_back(2);
            node5.add_dht_address(255,tmp_path);
            tmp_path.clear();


            node5.set_world(this);


    trace m_trace;
    m_trace.home_address  	= 1;
    m_trace.target_address 	= 5;
    world_of_nodes.insert(pair<ID_addr,dht_node>(node1.get_phisical_addr(),node1));
    world_of_nodes.insert(pair<ID_addr,dht_node>(node2.get_phisical_addr(),node2));
    world_of_nodes.insert(pair<ID_addr,dht_node>(node3.get_phisical_addr(),node3));
    world_of_nodes.insert(pair<ID_addr,dht_node>(node4.get_phisical_addr(),node4));
    world_of_nodes.insert(pair<ID_addr,dht_node>(node5.get_phisical_addr(),node5));



    node1.start_tracking(5);
/*
    for (int i=0; i<size ; i++) {

		dht_node tmp_node(calculate_hash(i));
		tmp_node.set_phisical_addr(i);
		world_of_nodes.insert(pair< ID_addr,dht_node>(calculate_hash(i),tmp_node));

    }
*/
/*
 *
 * 		tutaj dopisac dodawanie znanych nodow
	    for (int i=0 ;i<size; i++){
	       auto node_tmp = world_of_nodes.at(i);
	       {
	            list<phisical_addr> list ;
	            list.push_back((i+1)%size);
	            list.push_back((i+2)%size);
                list.push_back((i+3)%size);

	       }

	    }
*/
}


list<phisical_addr> dht_world::generete_route(phisical_addr home, phisical_addr dest) {
	list <phisical_addr> tmp_list;
	size_t world_size = world_of_nodes.size();

 // generowanie sciezki - np. takiej by dodac ja do  znanych tras

 // kazda trasa liczbe punktow taka jak 1/4 odleglosci ( 4 punkty na trase) - zeby zasymolowac przechodzenie pakietu przez hardwerowe adresy



	if(dest - home) {

	}


	return tmp_list;
}

void dht_world::send(phisical_addr addr,trace m_trace) {
    try{
	for(auto it:world_of_nodes) {
		if(it.second.get_phisical_addr() == addr) {
			it.second.recive(m_trace);
		}
    }
    }catch(...){

    }
}
