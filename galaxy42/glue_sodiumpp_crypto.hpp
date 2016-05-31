#pragma once
#ifndef glue_sodiumpp_crypto
#define glue_sodiumpp_crypto

#include "crypto.hpp"

#include <sodiumpp/sodiumpp.h>
#include "libs0.hpp"
#include "strings_utils.hpp"

namespace antinet_crypto {


template <unsigned int sequentialbytes>
std::string show_nice_nonce(const sodiumpp::nonce<sequentialbytes> & thenonce) {
	auto str = thenonce.get().to_binary();
	auto str_size = str.size();
	if (!(str_size<1024)) {
		// TODOthrow
		_erro("Strange size of (entire) nonce (size=" << str_size << ")");
		assert(false);
	}

	int num_zero = 0; // count the leading zeros
	size_t pos = 0;
	while (pos < str_size) {
		if (str.at(pos) == 0) ++num_zero; // another 0
		else break;
		++pos;
	}
	uint64_t ev=0; // the value at end
	for (size_t i=pos; i<str_size; ++i) {
		if (ev >= std::numeric_limits<int>::max() / 256) { // about to overflow at next <<8
			_erro("Strange large value of nonce, stopped at ev="<<ev<<" at i="<<i);
			return "(error)";
		}
		ev = (ev<<8) + static_cast<unsigned char>(str.at(i));
	}

	ostringstream oss;
	if (num_zero>0) {
		oss << "(0*" << num_zero << "),";
	}
	oss<<ev;
	return oss.str();
}


}	// namespace antinet_crypto
#endif


