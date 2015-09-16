#ifndef NEARESTFUNCT_H
#define NEARESTFUNCT_H

#include "headers.h"


class nearestFunct2 {
  private:

	ID_addr address;
	unsigned int popcount64(unsigned long long x);

  public:

	nearestFunct2():address(0) {
		;
	}
	nearestFunct2(ID_addr addr):address(addr) {
		;
	}
	void set_Params(ID_addr addr);

	bool operator ()(const std::pair<ID_addr,list <phisical_addr> > &left,const std::pair<ID_addr,list <phisical_addr> > &right);

    bool operator ()(const ID_addr left, const ID_addr);
};


class nearestFunct {
  private:

	ID_addr Address;

  public:

	nearestFunct(): Address(0) {
		;
	}
	nearestFunct(ID_addr addr):Address(addr) {
		;
	}
	void setParams(ID_addr addr) {
		Address = addr;
	}

	bool operator ()(const std::pair<ID_addr,list <phisical_addr> > &left,const std::pair<ID_addr,list <phisical_addr> > &right);

};


#endif // NEARESTFUNCT_H
