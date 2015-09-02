#include "hash_table.hpp"
unsigned hash_table::Capacities[28] = {2, 5, 11, 23, 47, 97, 197, 397, 797, 1597, 3203, 6421, 12853, 25717, 51437, 102877, 205759, 411527, 823117, 1646237, 3292489,
	6584983, 13169977, 26339969, 52679969, 105359939, 210719881, 421439783};

#define capacity Capacities[capacity_id]

unsigned hash_table::hash_func (const string &data, unsigned mod) const {
	unsigned int a = 63689;
	static const unsigned int b = 378551;
	unsigned long long hash = 0;

	for (char i : data) {
		hash = hash * a + static_cast<unsigned char>(i);
		a *= b;
	}

	return hash % mod;
}

void hash_table::rehash (bool change_size) {
	unsigned new_capacity = change_size ? Capacities[capacity_id + 1] : capacity;
	unsigned long long result;
	node *New_array = new node[new_capacity];

	for (unsigned i = 0; i < capacity; ++i) {
		if (Array[i].value != "") {
			for (result = hash_func (Array[i].value, new_capacity);
			     New_array[result].value != "";
			     result += ((result * new_capacity) % (new_capacity - 1) + 1), result %= new_capacity);

			New_array[result].value = Array[i].value;
		}
	}

	if (change_size)
		++capacity_id;

	delete [] Array;
	Array = New_array;
	removed = 0;
}

hash_table::hash_table () : ratio(0.65), size(0), removed(0), capacity_id(5) { Array = new node[capacity]; }

hash_table::~hash_table () { delete [] Array; }

void hash_table::insert (const string &val) {
	long long found = find (val);
	if (found != -1)
		return;

	if ((double)size / (double)capacity >= ratio)
		rehash (true);

	else if ((double)removed / (double)capacity >= 0.5*ratio)
		rehash (false);

	register unsigned long long result;
	for (result = hash_func (val, capacity);
	     Array[result].value != "" && Array[result].value != val;
	     result += ((result * capacity) % (capacity - 1) + 1), result %= capacity);

	if (Array[result].value != val) {
		if (Array[result].was_filled) {
			Array[result].was_filled = false;
			--removed;
		}
		Array[result].value = val;
		++size;
	}

	else
		Array[result].value = val;
}

long long hash_table::find (const string &val) const {
	register unsigned long long result;
	for (result = hash_func (val, capacity);
	     Array[result] && Array[result].value != val;
	     result += ((result * capacity) % (capacity - 1) + 1), result %= capacity);

	return (Array[result].value == val ? result : -1);
}

void hash_table::remove (const string &val) {
	register unsigned long long result;
	for (result = hash_func (val, capacity);
	     Array[result] && Array[result].value != val;
	     result += ((result * capacity) % (capacity - 1) + 1), result %= capacity);

	if (Array[result]) {
		Array[result].value = "";
		Array[result].was_filled = true;
		--size;
		++removed;
	}
}

void hash_table::clear () {
	delete [] Array;
	size = removed = 0, capacity_id = 5;
	Array = new node[capacity];
}

#undef capacity