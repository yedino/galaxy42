#ifndef C_LOG_HPP
#define C_LOG_HPP
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

using std::cout;
using std::cerr;
using std::endl;

namespace Log {
		template <typename T>
		inline void Debug (const T &arg) { cout << arg << endl; }

		template<typename T, typename... Args>
		inline void Debug (T first, Args... args) {
			cout << "debug: " << first;
			Debug(args...);
		}



		template <typename T>
		inline void Info (const T &arg) { cout << arg << endl; }

		template<typename T, typename... Args>
		inline void Info (T first, Args... args) {
			cout << first;
			Info(args...);
		}



		template <typename T>
		inline void Warning (const T &arg) { cerr << arg << endl; }

		template<typename T, typename... Args>
		inline void Warning (T first, Args... args) {
			cerr << first;
			Warning(args...);
		}



		template <typename T>
		inline void Error (const T &arg) { cerr << arg << endl; }

		template<typename T, typename... Args>
		inline void Error (T first, Args... args) {
			cerr << first;
			Error(args...);
		}
}


#endif // C_LOG_HPP

