#include "stats.hpp"

namespace ptest {
    stats_t::stats_t () { reset(); }

    void stats_t::reset () {
      exceptions = 0;
      failed = 0;
      passed = 0;
      timeout = 0;
      total_time = std::chrono::microseconds::zero();
    }
}