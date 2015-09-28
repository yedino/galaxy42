#ifndef NEARESTFUNCT_H
#define NEARESTFUNCT_H

#include "headers.h"


class nearestFunct2 {
  private:

	dht_addr address;
	unsigned int popcount64(unsigned long long x);

  public:

	nearestFunct2():address(0) {
		;
	}
	nearestFunct2(dht_addr addr):address(addr) {
		;
	}
	void set_Params(dht_addr addr);

	bool operator ()(const std::pair<dht_addr,list <phisical_addr> > &left,const std::pair<dht_addr,list <phisical_addr> > &right);

    bool operator ()(const dht_addr left, const dht_addr);
};


class nearestFunct {
  private:

	dht_addr Address;

  public:

	nearestFunct(): Address(0) {
		;
	}
	nearestFunct(dht_addr addr):Address(addr) {
		;
	}
	void setParams(dht_addr addr) {
		Address = addr;
	}

	bool operator ()(const std::pair<dht_addr,list <phisical_addr> > &left,const std::pair<dht_addr,list <phisical_addr> > &right);

};


#endif // NEARESTFUNCT_H
