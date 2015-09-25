#include <iostream>
#include <vector>
#include <string>
#include <memory>

template <typename T, typename U>
T& unique_cast_ref(std::unique_ptr<U> & u) noexcept {
	return dynamic_cast<T&>( * u.get() );
}

template <typename T, typename U>
T* unique_cast_ptr(std::unique_ptr<U> & u) noexcept {
	return dynamic_cast<T*>( u.get() );
}

struct animal { int age=1; virtual ~animal()=default; };
struct cat  : public animal { int lives=9; };

int main() {
	std::unique_ptr< animal > a_ptr( new cat );

	cat & c_ref = unique_cast_ref<cat>( a_ptr );
	cat * c_ptr = unique_cast_ptr<cat>( a_ptr );

	std::cerr << a_ptr->age << std::endl;
	std::cerr << c_ptr->age << std::endl;
	std::cerr << c_ptr->lives << std::endl;
	std::cerr << c_ref.age << std::endl;
	std::cerr << c_ref.lives << std::endl;
}

