#ifndef HEADERS
#define HEADERS

#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

typedef long long int dht_addr;				///hashed addres
typedef std::string phisical_addr;
typedef pair <dht_addr,phisical_addr> node_address;		///hashed address , node phisical address

struct trace{
    bool direction;					//null -direction home/else target

	dht_addr home_address;
	dht_addr target_address;
	dht_addr next_dht_address;

	list<phisical_addr> trace_to_next_dht;

	vector<phisical_addr> home_to_target_path;
    vector<phisical_addr> target_to_home_path;

    virtual ~trace() = default;

};

struct hello_trace_packet
{
	bool direction;
	phisical_addr target_addres;
	phisical_addr home_address;
	dht_addr target_dht_addres;
	dht_addr home_dht_address;		//value to set
	map<dht_addr ,list<phisical_addr> > known_nodes;
};

struct : public trace{

	dht_addr destination;
    list<phisical_addr> path_to_asked_node;


}typedef nodes_path;



#endif // HEADERS

