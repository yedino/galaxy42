#ifndef __LIBMATH__
#define __LIBMATH__

#include <vector>
#include <cstdlib>
#include <cmath>
#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>

/*
 * returns all coprimes to number in [begin, end]
 */
template <typename T>
std::vector<T> coprimes (T begin, T end, T number) {
	std::vector<T> result;
	T ax, bx, t;
	for (T i = begin; i <= end; ++i) {
		ax = i;
		bx = number;
		while (bx) {
			t = bx;
			bx = ax % bx;
			ax = t;
		}
		if (ax == 1)
			result.push_back(i);
	}
	return result;
}


/*
 * calculates (a * b) % c taking into account that a * b might overflow
 */
template <typename T>
T mult_modulo (T a, T b, T mod) {
	T x = 0, y = a % mod;
	while (b > 0) {
		if ((b & 1))
			x = (x + y) % mod;

		y = (y << 1) % mod;
		b >>= 1;
	}
	return x % mod;
}


/*
 * calculates (a ^ b) % mod taking into account that a * b might overflow
 */
template <typename T>
T power_modulo (const T &a, const T &b, const T &mod) {
	T result = 1;
	T x = a % mod;

	for (T i = 1; i <= b; i <<= 1) {
		x %= mod;
		if (b & i) {
			result *= x;
			result %= mod;
		}
		x *= x;
	}
	return result;
}


/*
 * Miller-Rabin primarity test
 */
template <typename T>
bool is_prime (const T &p, int accuracy = 20) {
	if (p <= 2)
		return (p == 2);

	if (!(p & 1))
		return false;

	T s = p - 1;
	while (!(s & 1))
		s >>= 1;

	T a, mod, temp;
	for (T i = 0; i < accuracy; ++i) {
		a = rand() % (p - 1) + 1, temp = s;
		mod = power_modulo(a, temp, p);
		while (temp != p - 1 && mod != 1 && mod != p - 1) {
			mod = mult_modulo(mod, mod, p);
			temp <<= 1;
		}
		if (mod != p - 1 && !(temp & 1))
			return false;
	}
	return true;
}


/*
 * calculates inverse of a mod mod
 * notice, that its non-throw function!
 * e has to be coprime with mod
 */
template <typename T>
T mult_inverse (T e, T mod) {
	T mod_bckp = mod, t, q;
	T x0 = 0, result = 1;
	if (mod == 1)
		return 1;

	while (e > 1) {
		q = e / mod;
		t = mod, mod = e % mod, e = t;
		t = x0, x0 = result - q * x0, result = t;
	}
	if (result < 0)
		result += mod_bckp;

	return result;
}


/*
 * computes all prime numbers in [0, end]
 */
//std::vector<long long> primes (long long end) {
//        long long g, x_2, y_2;
//        bool *S = new bool[end + 1];
//        for (long long i = 5; i <= end; ++i)
//                S[i] = false;

//        g = (long long)sqrt((double)end);
//        for (long long x = 1; x <= g; ++x) {
//                x_2 = x * x;
//                for (long long y = 1, z; y <= g; ++y) {
//                        y_2 = y * y;
//                        z = (x_2 << 2) + y_2;
//                        if ((z <= end) && ((z % 12 == 1) || (z % 12 == 5)))
//                                S[z] = !S[z];

//                        z -= x_2;
//                        if ((z <= end) && (z % 12 == 7))
//                                S[z] = !S[z];

//                        if (x > y) {
//                                z -= y_2 << 1;
//                                if ((z <= end) && (z % 12 == 11))
//                                        S[z] = !S[z];
//                        }
//                }
//        }

//        for (long long i = 5, z; i <= g; ++i) {
//                if (S[i]) {
//                        x_2 = i * i;
//                        z = x_2;
//                        while (z <= end) {
//                                S[z] = false;
//                                z += x_2;
//                        }
//                }
//        }

//        std::vector<long long> result = {2, 3};
//        for (long long i = 5; i <= end; ++i) {
//                if (S[i])
//                        result.push_back(i);
//        }
//        delete[] S;
//        return result;
//}


/*
 * uses Fermat's little theorem to test whether p is non-prime number
 */
template <typename T>
bool is_non_prime (const T &p, int accuracy = 10) {
	if (p < 6)
		return false;

	if (p % 5 == 0)
		return true;

	T a;
	for (short i = 0; i < accuracy; ++i) {
		a = rand() % (p - 5) + 2;
		if (power_modulo(a, p - 1, p) != 1) {
			return true;
		}
	}
	return false;
}

/*
 * calculates gcd(a,b)
 */
template <typename T>
T gcd (T a, T b) {
	T k = 0, r;
	while (!((a | b) & 1)) {
		a >>= 1, b >>= 1;
		++k;
	}

	while (b != 0) {
		if (!a) {
			return b << k;
		}

		while (!(a & 1))
			a >>= 1;

		while (!(b & 1))
			b >>= 1;

		if (a >= b) {
			a = (a - b) >> 1;
		} else {
			r = (b - a) >> 1, b = a, a = r;
		}
	}
	return a << k;
}

/*
 * calculates euler's function phi(p*q)
 */
template <typename T>
T euler_func (const T &p, const T &q) { return T((p - 1) * (q - 1)); }

#endif // __LIBMATH__
