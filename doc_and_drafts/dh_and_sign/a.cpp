/* 
On the BSD Licence by tigusoft.pl

Warning
This is a toy / test program, absolutelly do not use it for anything important nor for any real crypto!!! XXX

This code is totally wrong now, because
- useless random
- where to use modulo p? TODO
- other requirements, limits and corner cases
- timing attacks and other side-channel
- other bugs :)

*/

#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "libmath.hpp"

#include <boost/multiprecision/cpp_int.hpp>
using namespace boost::multiprecision;

cpp_int big_random(int digits) {
	cpp_int x;
	for (auto i=0; i<digits; ++i) x = x*10 + rand()%10;
	return x;
}

template <typename T> 
T hash_of_two(T a, T b) {
	// consts should be big primes here
	T x = ( a * 906259 + b * 1201939 + a * 96201247 + b * 58207187 + (a%185869)*(b%301489) ) % 295531109;
	return x;
}

using namespace std;

// using big_pow = boost::multiprecision::pow;

void run_test(bool dbg);

int main() {
	srand(time(NULL));

	for (int i=0; i<50; i++) {
		run_test(1);
		cout << endl;
	}
	run_test(1);
}

void run_test(bool dbg) {
	const int len = 20;
	cpp_int p = big_random(len);
	// *cpp_int("1000000000000000000000000000000000000000000");

	cpp_int g = 117;

	cpp_int a = big_random(len);
	cpp_int A = power_modulo( g , a , p );

	cpp_int b = big_random(len);
	cpp_int B = power_modulo( g , b , p );

	if (dbg) cout << "a=" << a << endl;
	if (dbg) cout << "A=" << A << endl;

	if (dbg) cout << "b=" << b << endl;
	if (dbg) cout << "B=" << B << endl;

	cpp_int K_a = power_modulo( B , a , p);
	cpp_int K_b = power_modulo( A , b , p);

	cout << "Alice: K=" << K_a << endl;
	if (dbg) cout << "Bob:   K=" << K_a << endl;

	// Signature:
	// Signature is: r,e,s where
	// 1) k=secret random (different for each signature).
	// 2) r=g^k
	// 3) e=hash(M+r)
	// 4) s=k-a*e

	cpp_int M = 123456789;

	cpp_int k = big_random(len*2) + boost::multiprecision::pow(cpp_int(10),len);
	cpp_int r = power_modulo(g , k , p);
	cpp_int e = hash_of_two(M,r) % p;
	//cpp_int s = k - (a*e)%p % p;
	cpp_int s = k - (a*e); // <<<--- when to use modulo?

	if (dbg) cout << "k  =" << k << endl;
	if (dbg) cout << "r  =" << r << endl;
	if (dbg) cout << "e  =" << e << endl;
	if (dbg) cout << "s  =" << s << endl;

	cout << endl << "Signature is {" << s << "," << e << "," << r << "} " << "(k="<<k<<") on M="<<M<<"   ";
	if (dbg) cout<<endl;

	// calculate our r' =?= g^s * A^e - and then check if r' = r 

	cpp_int r_verif = ( power_modulo(g,s,p) * power_modulo(A,e,p) ) % p;
	cout << "r' =" << r_verif ;
	if (r == r_verif) cout << " OK "; else cout << " ERROR (r="<<r<<") !!! ";
	cout << endl;

}


