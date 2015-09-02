#ifndef __CUSTOM__HASH_TABLE__
#define __CUSTOM__HASH_TABLE__

#include <cstdlib>
#include <string>
#include <cmath>

using std::string;

class hash_table { // == AND .key()
private:
	class node {
	public:
		bool was_filled;
		string value;

		node () : was_filled(false), value("") { }

		operator bool () { return (value != "") || was_filled; }
	};

	node *Array;
	double ratio;
	unsigned size, removed;
	unsigned short capacity_id;
	static unsigned Capacities[28];

	unsigned hash_func (const string &data, unsigned mod) const;

	void rehash (bool change_size);

public:
	hash_table ();

	~hash_table ();

	void insert (const string &val);

	long long find (const string &val) const;

	void remove (const string &val);

	void clear ();
};

#endif // __CUSTOM__HASH_TABLE__