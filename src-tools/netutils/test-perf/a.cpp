#include <iostream>
#include <vector>

double mytest2_long() {
	double a=1;
	for (int i=0; i<1000*1000*50; ++i) {
		a += a*0.1 + i*0.2;
	}
	return a;
}

double mytest2_short() {
	double a=2;
	for (int i=0; i<1000*1000*10; ++i) {
		a += a*0.101 + i*0.202;
	}
	return a;
}


void mytest2() {
	double d = mytest2_long() + mytest2_short();
	std::cout << d << std::endl;
}

void mytest1() {
	mytest2();
}

void test_vector_byte() {
	std::vector<unsigned char> tab;
	for (long int i=0; i<10*1000*1000; ++i) tab.push_back( i%100 );
	std::cout << tab.size() << std::endl;
}

void test_vector_long() {
	std::vector<long int> tab;
	for (long int i=0; i<10*1000*1000; ++i) tab.push_back( i%100 );
	std::cout << tab.size() << std::endl;
}

void mytest_vector() {
	test_vector_byte();
	test_vector_long();
}

int main() {
	mytest1();
	mytest1();
	mytest1();
	mytest_vector();
}

