#ifndef PLIB_PTEST_CONFIG_HPP
#define PLIB_PTEST_CONFIG_HPP

#include <unordered_map>
#include <chrono>
#include <mutex>

namespace ptest {
    enum class output_type { // WHY enum clases can't inherit ?!?!?!?////!!!?!1111111111
        failed = 0,
        passed = 1,
        timeout = 2,
        exception = 3,
        info = 4
    };

    // output_type hasher
    struct hash {
        size_t operator() (const ptest::output_type &o) const;
    };

    struct config_t {
        std::unordered_map<output_type, std::reference_wrapper<std::ostream>, hash> outputs;
        std::chrono::microseconds max_time;
        bool is_timeout_active = false;

        bool print_passed_tests = false;
        bool print_exceptions = true;
        bool print_names_of_arguments = true;

        bool terminate_after_first_failure = false;
        bool terminate_after_first_exception = false;
        bool terminate_after_first_timeout = false;

        config_t ();

        void set_def_outputs ();

        void set_all_output_to (std::ostream &);

        template <class Rep, class Period = std::ratio<1>>
        void set_max_time (const std::chrono::duration<Rep, Period> &max) {
          if (max > std::chrono::milliseconds::max() - std::chrono::milliseconds(15)) {
            throw std::logic_error("invalid max_time set");
          }
          max_time = max;
          is_timeout_active = true;
        }
    };
}


#endif //PLIB_PTEST_CONFIG_HPP
