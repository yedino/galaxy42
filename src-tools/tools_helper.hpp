#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>

template<typename TFloat>
TFloat mediana(std::vector<TFloat> tab) {
	if (tab.size()<=0) return 0;
	assert(tab.size()>=1);

	// std::nth_element(tab.begin(), tab.begin() + tab.size()/2, tab.end()); // odd

	sort(tab.begin(), tab.end());

	if (1==(tab.size() % 2 )) {
			return tab.at( tab.size()/2 ); // mediana odd    [10 *20 30]
	} else {
		auto a = tab.at( tab.size()/2 -1 );
		auto b = tab.at( tab.size()/2  );

		return (a+b)/2.;
		// mediana odd    [10 *20 *30 40]
	}
}

template<typename TFloat>
TFloat corrected_avg(std::vector<TFloat> tab) {
	auto size = tab.size();
	if (size<=0) return 0;
	if (size<=1) return tab.at(0);

	auto margin = size/4;
	if (margin<1) margin=1;

	auto pos1 = 0+margin, pos2=size-1-margin;
	if (pos2 < pos1) return 0; // too small; can't substract
	int len = pos2-pos1;
	if (!(len>=2)) return 0; // too small yet

	//	std::cerr<<"Avg from "<<pos1<<" to " <<pos2 << " len="<<len<<" size="<<size<<std::endl;

	return std::accumulate( & tab.at(pos1) , & tab.at(pos2) , 0 ) / len;
}

