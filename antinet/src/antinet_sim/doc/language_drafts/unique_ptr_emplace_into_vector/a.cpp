#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <chrono>

// --- intro ---
/*  
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
//		stringEater3(std::move(tmpString));
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Printing took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms.\n";
*/


namespace std{
// http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
template <typename T, typename... Args>
std::unique_ptr<T> make_unique (Args &&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}

template <typename T, typename U>
T& unique_cast_ref(std::unique_ptr<U> & u) noexcept {
	return dynamic_cast<T&>( * u.get() );
}

template <typename T, typename U>
T* unique_cast_ptr(std::unique_ptr<U> & u) noexcept {
	return dynamic_cast<T*>( u.get() );
}

// #define _info(X) do { std::cerr<<"info: " << X << std::endl; } while(0) 
#define _info(X)
#define _mark(X) do { std::cerr<<"info: " << X << std::endl; } while(0) 

std::string g_name="foooobaaaaaaaaaaaaaaarlonganimalname";

struct animal { 
	int age=1; 
	long double mana=100.0;

	std::string m_name;

	virtual ~animal()=default; 


	animal(const std::string & name): m_name(name)  { /*   _info(this<<" T("<<m_name<<")");*/ } 
	animal(std::string && name): m_name(std::move(name))  { /* _info(this<<" T("<<m_name<<")");*/ } 

	animal(const animal & obj) : age(obj.age), mana(obj.mana), m_name(obj.m_name) { /*_info(this<<" T() copy")*/; }
	animal(animal && obj) { // _info(this<<" T() move"); 
		this->age = obj.age;
		this->mana = obj.mana;
		this->m_name = std::move(obj.m_name);
	} 

	animal& operator=(const animal & obj) { /*  _info(this<<" op= copy")*/; 
		age=obj.age; 
		mana=obj.mana;
		m_name=obj.m_name;
		return *this; 
	}

	animal& operator=(animal && obj) {/* _info(this<<" op= move")*/; 
		age=obj.age; 
		this->age = obj.age;
		this->mana = obj.mana;
		this->m_name = std::move(obj.m_name);
		return *this; 
	}

};

struct cat  : public animal { int lives=9; };

// --- main problem ---

struct world {
	std::vector< std::unique_ptr< animal > > m_animal_ptr;
	std::vector< animal > m_animal_obj;
	world(){m_animal_ptr.reserve(5000);m_animal_obj.reserve(5000);}


	void add_ptr(std::unique_ptr<animal> && ptr) {
		m_animal_ptr.emplace_back( std::move(ptr) ); // move the unique_ptr to the animal into our array
	}

	void add_obj(animal && obj) {
		m_animal_obj.emplace_back( std::move(obj) ); // move the object of the animal into our array
	}

	void new_obj() {
		m_animal_obj.emplace_back( g_name  ); // move the object of the animal into our array
	}

	void new_ptr() {
		m_animal_ptr.emplace_back( std::make_unique<animal>(g_name) ); //
	}

};

int main_test() {

	auto & name = g_name;

	long int number=5000; 
	long int tries=5000;

		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	for (long int t=0; t<tries; ++t)
	{
		world w;
		for (long int n=0; n<number; ++n) {
			std::unique_ptr< animal > a( new animal(name) );
			w.add_ptr( std::move(a) );
		}
	}
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "add_ptr took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms.\n";

//	_mark("add_ptr: ");


		 start = std::chrono::steady_clock::now();
	for (long int t=0; t<tries; ++t)
	{
		world w;
		for (long int n=0; n<number; ++n) {
		//	_info("Test2");
			animal a(name);
			w.add_obj( std::move(a) );
		}
	}

     end = std::chrono::steady_clock::now();
    std::cout << "add_obj took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms.\n";

//	_mark("add_obj: ");


		 start = std::chrono::steady_clock::now();
	for (long int t=0; t<tries; ++t)
	{
		world w;
		for (long int n=0; n<number; ++n) {
//			_info("Test3");
			w.new_obj( );
		}
	}
     end = std::chrono::steady_clock::now();
    std::cout << "new_obj took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms.\n";
//	_mark("new_obj: ");



		 start = std::chrono::steady_clock::now();
	for (long int t=0; t<tries; ++t)
	{
		world w;
		for (long int n=0; n<number; ++n) {
//			_info("Test new_ptr");
			w.new_ptr( );
		}
	}
     end = std::chrono::steady_clock::now();
    std::cout << "new_ptr took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms.\n";

//	_mark("new_ptr: ");


}

int main() {

	for (int m=0; m<2; ++m) {
		_mark("---Testing---");
		main_test();
	}
};

