#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct c_dog {
		string name;
		c_dog(const string & n="Doge");
		void bark() const;
};
c_dog::c_dog(const string & n) : name(n) { }
void c_dog::bark() const { cout<<"Woof! " << name << endl; }

struct c_world {
		vector< c_dog > m_all_dogs; /// all existing dogs
		c_dog & create_dog();
};
c_dog & c_world::create_dog() {
	m_all_dogs.emplace_back();
	return m_all_dogs.back();
}

struct c_home {
		c_dog & m_dog; // reference to my dog
		c_home(c_world & world); ///< WARNING: world must outlive this object!
		c_home(c_dog & existing_dog); ///< WARNING: existing_dog must outlive (e.g. owned by some world) this object!
		void test();
};
c_home::c_home(c_world & world) : m_dog(world.create_dog()) { } 
c_home::c_home(c_dog & existing_dog) : m_dog(existing_dog) { } 
void c_home::test() {	m_dog.bark(); }

struct c_city {
	vector< c_home > m_home;
	void test1();
	void test2();

	void test3(c_world & world);
	void test4();
};

void c_city::test1() {
	c_world miniworld;
	c_home h(miniworld);
	m_home.push_back( h );
}

void c_city::test2() {
	for (c_home &h : m_home) h.test();
}

void c_city::test3(c_world & world) {
	{
		c_dog & puppy_ret = world.create_dog(); // OK
		c_dog puppy = puppy_ret; // bad! object copy

		c_home h(puppy); // it takes reference to local object
		h.test();

		m_home.push_back( h ); // oh no the home will continue to exist
	}	
	// h.test(); // not in scope - ok :) 

	m_home.back().test(); // <- home references a dead dog :'( - UB, but usually does not crash because function stack usually exists
}

void c_city::test4() {
	m_home.back().test(); // <- home references a dead dog :'( - UB, and usually will crash since the dead dog is now on removed function-stack
}

int main() {
	c_world earth;
	c_home myhome(earth);
	myhome.test();

	c_city Cracow;
	Cracow.test3(earth);
	Cracow.test4();
	//Cracow.test1();
	//Cracow.test2();
}

