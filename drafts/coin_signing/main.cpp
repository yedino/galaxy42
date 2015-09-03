#include <iostream>
#include <list>

using namespace std;

class token_id_generator {
		static size_t id;
public:
		static size_t generate_id () { return id++; }
};
size_t token_id_generator::id = 0;



struct c_token {
		const size_t id;
		c_token () : id (token_id_generator::generate_id()) { }
};

struct c_wallet {
		list<c_token> tokens;
		const string tokens_type;
		c_wallet (const string &name) : tokens_type(name) { }
		c_wallet (string &&name) : tokens_type(name) { }
};

int main () {

	return 0;
}