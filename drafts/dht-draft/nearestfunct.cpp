#include "nearestfunct.h"


bool nearestFunct::operator()(const std::pair<ID_addr,list <phisical_addr> > &left, const std::pair<ID_addr,list <phisical_addr> > &right) {
	ID_addr distanceLeft = abs (left.first - Address)<abs(left.first - Address )?abs (left.first - Address):abs(left.first - Address );
	ID_addr distanceRight = abs (right.first - Address)<abs(right.first - Address )?abs (right.first - Address):abs(right.first - Address);

	return distanceLeft<distanceRight ;
}


unsigned int nearestFunct2::popcount64(unsigned long long x) { // to moze nie dzialac- funkcja z internetow
	x = (x & 0x5555555555555555ULL) + ((x >> 1) & 0x5555555555555555ULL);
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
	return (x * 0x0101010101010101ULL) >> 56;
}

bool nearestFunct2::operator()(const std::pair<ID_addr,list <phisical_addr> > &left, const std::pair<ID_addr,list <phisical_addr> > &right) {
//	unsigned int distance_left =	popcount64(address^left.first);
//	unsigned int distance_right =   popcount64(address^right.first);

    return 	this->operator ()(left.first,right.first);//distance_left<distance_right;
}
bool nearestFunct2::operator ()(const ID_addr left,const ID_addr right){
    unsigned int distance_left =	popcount64(address^left);
    unsigned int distance_right =   popcount64(address^right);

    return distance_left<distance_right;

}

