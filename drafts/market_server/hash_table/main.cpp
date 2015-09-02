#include <iostream>
#include "hash_table.hpp"
#include <vector>

using namespace std;

string randomm (int size) {
	static const char alphanum[] = "0123456789"
		"!@#$%^&*"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	static int stringLength = sizeof(alphanum) - 1;
	string result;
	for (int i = 0; i < size; ++i)
		result += alphanum[rand() % stringLength];

	return result;
}

int main () {
	vector<string> values, values2;
	hash_table h;
	for (int i = 0; i < 50000; ++i)
		values.push_back(randomm(10));

	for (int i = 0; i < 50000; ++i)
		values2.push_back(randomm(10));

	for (auto &v : values) {
		if (h.find(v) != -1) {
			cout << "error\n";
		}
		h.insert(v);
	}

	for (auto &v : values) {
		if (h.find(v) == -1) {
			cout << "error\n";
		}
	}

	for (auto &v : values) {
		h.remove(v);
	}

	for (auto &v : values) {
		if (h.find(v) != -1) {
			cout << "error\n";
		}
	}
	cout << "oK!";
	return 0;
}