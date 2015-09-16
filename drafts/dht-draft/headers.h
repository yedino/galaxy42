#ifndef HEADERS
#define HEADERS

#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

typedef long long int ID_addr;				///hashed addres
typedef unsigned long int phisical_addr;
typedef pair <ID_addr,phisical_addr> node_address;		///hashed address , node phisical address

struct trace{
    bool direction;					//null -direction home/else target

	ID_addr home_address;
	ID_addr target_address;
	ID_addr next_dht_address;

	list<phisical_addr> trace_to_next_dht;

	vector<phisical_addr> home_to_target_path;
    vector<phisical_addr> target_to_home_path;

    virtual ~trace() = default;

};

struct : public trace{

    ID_addr destination;
    list<phisical_addr> path_to_asked_node;


}typedef nodes_path;



#endif // HEADERS

