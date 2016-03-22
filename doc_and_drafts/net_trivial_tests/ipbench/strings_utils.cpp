
#include "strings_utils.hpp"

#include <libs1.hpp>

string_as_hex::string_as_hex(const std::string & s) : data(s) { }

unsigned char hexchar2int(char c) {
	if ((c>='0')&&(c<='9')) return c-'0';
	if ((c>='a')&&(c<='f')) return c-'a' +10;
	throw std::invalid_argument("Invalid character in parsing hex number");
}

string_as_bin::string_as_bin(const string_as_hex & encoded) {
	// "ff020a" = ff , 02 , 0a
	//   "020a" = 02 , 0a
	//    "20a" = 02 , 0a
	_info("Processing string: ["<< (encoded.data) << "]");
	const auto es = encoded.data.size();
	if (!es) return; // empty string

	size_t retsize = es/2; // size of finall string of bytes data
	if (0 != (es % 2)) retsize++;
	assert(retsize > 0);
	assert( (retsize < es) || (retsize==es==1) ); // both are ==1 for e.g. "a"
	bytes.resize(retsize);

	size_t pos=0, out=0; // position of input, and output
	for( ; pos<es ; pos+=2, ++out) {
		// _info("pos="<<pos<<" out="<<out<<" encoded="<<encoded.data);
		// "02" -> cl="2" ch="0"
		//  "2" -> cl="2" ch="0"
		char cl,ch;
		if (pos+1 < es) { // pos and pos+1 are valid positions in string
			ch = encoded.data.at(pos);
			cl = encoded.data.at(pos+1);
		} else {
			ch = '0';
			cl = encoded.data.at(pos);
		}
		unsigned char octet = hexchar2int(ch)*16 + hexchar2int(cl);
		bytes.at(out) = octet;
	}

	assert( out == retsize ); // all expected positions of data allocated above in .resize() were written
}

