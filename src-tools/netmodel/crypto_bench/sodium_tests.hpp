#include <iostream>
#include <sodium.h>
#include <algorithm>
#include <numeric>
#include <vector>
#include <thread>
#include <fstream>
#include <chrono>
#include <array>
#include <cmath>
#include <condition_variable>
#include <mutex>
#include <stdplus/affinity.hpp>



template<typename F, size_t max_buff_size = 65536, size_t max_threads_count = 16>
class crypto_test {
public:
    crypto_test(F fun, std::string file_name);
    void test_buffer_size(std::function<void(unsigned char*, unsigned char*, size_t, size_t)>);
private:
    const F m_test_fun;
    std::ofstream m_file;
};

template<typename F, size_t max_buff_size, size_t max_threads_count>
crypto_test<F, max_buff_size, max_threads_count>::crypto_test(F fun, std::string file_name) : m_test_fun(fun), m_file(file_name)
{

}

template<typename F, size_t max_buff_size, size_t max_threads_count>
void crypto_test<F, max_buff_size, max_threads_count>::test_buffer_size(std::function<void(unsigned char*, unsigned char*, size_t, size_t)> init_buffer_lambda)
{
    std::array<size_t, max_threads_count> thread_results; //tests results from threads in ms
    std::array<std::thread, max_threads_count> threads;
	std::array<std::condition_variable, max_threads_count> thread_cv;
	std::array<bool, max_threads_count> thread_ready_flag;
	std::condition_variable trigger_cv; // if notifyed all threads starts
	std::mutex thread_mutex;
	bool threads_started = false; // protected by thread_mutex
    constexpr size_t iterations = 1000000;
	const size_t number_of_cpu = std::thread::hardware_concurrency();

    auto thread_lambda = [&init_buffer_lambda, &thread_results, &iterations, this, &thread_cv, &thread_mutex, &threads_started, &trigger_cv, &thread_ready_flag, &number_of_cpu]
	(size_t buff_size, size_t return_array_index, size_t number_of_concurrent_threads) {
        try {
            stdplus::affinity::set_current_thread_affinity(return_array_index % number_of_cpu);
        }
        catch (const std::exception & ex) {
            std::cerr << "Can not set CPU: " << ex.what();
        }

        std::vector<unsigned char> inbuff(buff_size);
        std::vector<unsigned char> outbuff(buff_size);
        init_buffer_lambda(inbuff.data(), outbuff.data(), buff_size, buff_size);

		{
		std::unique_lock<std::mutex> lg(thread_mutex);
		thread_ready_flag.at(return_array_index) = true;
		thread_cv.at(return_array_index).notify_all(); // i am readry to work
		trigger_cv.wait(lg, [&threads_started]{return threads_started;}); // wait for start
		}

        auto start_point = std::chrono::steady_clock::now();
        for (size_t j=0; j < iterations / number_of_concurrent_threads; j++)
        {
            m_test_fun(inbuff.data(), outbuff.data(), buff_size, max_buff_size);
        }
        auto end_point = std::chrono::steady_clock::now();
        thread_results.at(return_array_index) = std::chrono::duration_cast<std::chrono::nanoseconds>(end_point - start_point).count();
    };


    for (size_t ii=4096; ii < max_buff_size;) // iterate through buffer size
    {
        if (ii>=256)
            ii *= 1.5;
        else
            ii++;

        size_t i = ii;
        if (i > 256) {
          i = (i/64)*64;
          // no need to remove dupicate for this particular parametes ( *= 1.5 for > 256 , round to 64)
        }

        //for (size_t threads_count=0; threads_count<max_threads_count; threads_count++) // iterate through threads count
        {
            size_t threads_count = 8;
            thread_results.fill(0);
			thread_ready_flag.fill(false); // all threads are not ready
            for (size_t j=0; j<=threads_count; j++) // iterate through threads count
            {
                threads.at(j) = std::thread(thread_lambda, i, j, threads_count+1);
            }

			for (size_t j=0; j<=threads_count; j++) { // waiting for all threads
				std::unique_lock<std::mutex> lg(thread_mutex);
				thread_cv.at(j).wait(lg, [&thread_ready_flag, &j]{return thread_ready_flag.at(j);});
			}

			{ // run all threads
				std::unique_lock<std::mutex> lg(thread_mutex);
				threads_started = true;
			}
			auto start_point = std::chrono::steady_clock::now();
			trigger_cv.notify_all();


            for (size_t j=0; j<=threads_count; j++) // iterate through threads count
            {
                threads.at(j).join();
            }
		auto stop_point = std::chrono::steady_clock::now();
		threads_started = false;
		size_t time_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(stop_point - start_point).count();
		m_file << i << '\t' << threads_count << '\t' << time_ms/static_cast<double>(iterations) << '\n';
            //size_t time_ms = std::accumulate(thread_results.begin(), thread_results.end(), 0);
            //m_file << i << '\t' << threads_count+1 << '\t' << (time_ms/static_cast<double>(iterations))/((threads_count+1)*(threads_count+1))<< '\n'; // buff_size threads_count time_ms
            // threads_count+1 becouse array index starts from 0

        }
        std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << i << '/' << max_buff_size << std::flush;
    }
}
