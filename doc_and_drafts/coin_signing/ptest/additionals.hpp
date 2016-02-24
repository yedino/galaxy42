#ifndef PLIB_PTEST_ADDITIONALS_HPP
#define PLIB_PTEST_ADDITIONALS_HPP

#include <chrono>
#include <thread>
#include <list>

namespace ptest {
    class timeout_exception : public std::exception {
        virtual const char *what () const throw();
    };

    template <typename func_t, typename ... Args>
    std::chrono::microseconds measure_execution_time (func_t &&func, Args &&... args) {
      auto start = std::chrono::high_resolution_clock::now();
      func(std::forward<Args>(args)...);
      auto end = std::chrono::high_resolution_clock::now();
      return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    template <typename func_t, typename ... Args>
    void call_test (size_t threads_number, func_t &&func, Args &&... args) {
      std::list<std::thread> threads;
      for (size_t i = 0; i < threads_number; ++i)
        threads.emplace_back(std::thread(func, std::forward<Args>(args)...));

      for (auto &t : threads)
        t.join();
    }
}

#endif //PLIB_PTEST_ADDITIONALS_HPP
