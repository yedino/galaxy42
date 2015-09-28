#ifndef DHT_WORLD_H
#define DHT_WORLD_H

#include "headers.h"
#include "dhtnode.h"

class dht_node;

class dht_world {
	map <dht_addr,dht_node> world_of_nodes;

	dht_addr calculate_hash(phisical_addr addr);

  public:
	dht_world();
	void create_world(size_t size);

	void send(phisical_addr addr,trace m_trace);

	list<phisical_addr> generete_route(phisical_addr home, phisical_addr dest);

};

#endif // DHT_WORLD_H
