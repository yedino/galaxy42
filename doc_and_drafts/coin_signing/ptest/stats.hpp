#ifndef PLIB_PTEST_STATS_HPP
#define PLIB_PTEST_STATS_HPP

#include <chrono>
#include <atomic>

namespace ptest {
    struct stats_t {
        std::atomic<size_t> passed;
        std::atomic<size_t> failed;
        std::atomic<size_t> timeout;
        std::atomic<size_t> exceptions;
        std::atomic<std::chrono::microseconds> total_time;

        void reset ();
        stats_t ();
    };
}

#endif //PLIB_PTEST_STATS_HPP
