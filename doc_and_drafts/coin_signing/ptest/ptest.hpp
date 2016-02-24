#ifndef PLIB_PTEST_HPP
#define PLIB_PTEST_HPP

#include <tuple>
#include <vector>
#include <chrono>
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <atomic>
#include <list>
#include <fstream>
#include <unordered_map>
#include "prettyprint/pretty_print.hpp"
#include "stats.hpp"
#include "config.hpp"
#include "additionals.hpp"

namespace ptest {
    class ptest_suite {
    public:
        mutable stats_t stats;
        static stats_t general_stats;
        config_t config;

        ptest_suite (const std::string &suite_name, const config_t &local_config = config_t());

        // =========================================================================

        template <typename func_t, typename T, typename ... Args>
        void start_test (func_t &&function,
                const std::string &func_name,
                const T &expected_result,
                bool equality,
                std::vector<std::string> &&args_names,
                Args &&... args) const {

          using return_type = typename std::result_of<func_t (Args...)>::type;
          // VOODOO MAGIC, DO NOT TOUCH
          static const std::string voodoo[2][5] = {{"  ", "\n", "!(", ")\n", "  "},
                                                   {"",   "\n", "",   "\n",  ""}};

          function_result <return_type> result;

          try {
            result = run_function(config.is_timeout_active, function, std::forward<Args>(args)...);
          } catch (const timeout_exception &ex) {
            print_test_preamble(output_type::timeout , func_name, args_names, std::forward<Args>(args) ...);
            print_thread_safe(output_type::timeout, "\n\tTIMEOUT\n\n");
            update_stats(function_status::timeout);
            if (config.terminate_after_first_timeout)
              terminate_process();

            return;
          } catch (const std::exception &exc) {
            if (config.print_exceptions) {
              print_test_preamble(output_type::exception , func_name, args_names, std::forward<Args>(args)...);
              print_thread_safe(output_type::exception, "\n\tEXCEPTION CAUGTH: ", exc.what(), "\n\n");
            }
            update_stats(function_status::exception);
            if (config.terminate_after_first_exception)
              terminate_process();

            return;
          } catch (...) {
            if (config.print_exceptions) {
              print_test_preamble(output_type::exception, func_name, args_names, args ...);
              print_thread_safe(output_type::exception, "\n\tEXCEPTION CAUGTH", "\n\n");
            }
            update_stats(function_status::exception);
            if (config.terminate_after_first_exception)
              terminate_process();

            return;
          }

          bool is_ok = (result.result == expected_result);
          if (!equality)
            is_ok = !is_ok;

          if (is_ok) {
            update_stats(function_status::passed, result.execution_time);
            if (config.print_passed_tests) {
              print_test_preamble(output_type::passed , func_name, args_names, args...);
              print_thread_safe(output_type::passed, "\n\tPASSED in ", result.execution_time.count(), " microseconds\n\n");
            }
          } else {
            update_stats(function_status::failed, result.execution_time);
            print_test_preamble(output_type::failed , func_name, args_names, args ...);
            print_thread_safe(output_type::failed, "\n\tFAILED in        ", voodoo[equality][4], result.execution_time.count(), " microseconds\n");
            print_thread_safe(output_type::failed, "\tresult was       ", voodoo[equality][0], result.result, voodoo[equality][1]);
            print_thread_safe(output_type::failed, "\texpecting result ", voodoo[equality][2], expected_result, voodoo[equality][3], '\n');
            if (config.terminate_after_first_failure)
              terminate_process();
          }
        }

        void run_assertion (bool expr, const std::string &name, const std::string &msg = "");

        // =========================================================================

        void print_suite_result () const;

        void print_general_result () const;

    private:
        template <typename T>
        struct function_result {
            T result;
            std::chrono::microseconds execution_time;
        };

        enum class function_status {
            failed = 0,
            passed = 1,
            timeout = 2,
            exception = 3
        };

        std::string suite_name;

        // =========================================================================

        template <typename func_t, typename ... Args>
        auto run_function_with_timeout (func_t &&func,
                Args &&... args) const -> function_result<typename std::result_of<func_t (Args...)>::type> {

          using return_type = typename std::result_of<func_t (Args...)>::type;

          auto start = std::chrono::high_resolution_clock::now();
          auto exec = [&func, &args...] () -> return_type {
              return func(std::forward<Args>(args)...);
          };

          auto task = std::packaged_task<return_type ()>(exec);

          auto handle = task.get_future();
          std::thread th(std::move(task));

          if (handle.wait_for(config.max_time + std::chrono::milliseconds(10)) == std::future_status::timeout) {
            th.detach();
            throw timeout_exception();
          } else {
            auto end = std::chrono::high_resolution_clock::now();
            th.detach();
            return {handle.get(), std::chrono::duration_cast<std::chrono::microseconds>(end - start)};
          }
        }

        template <typename func_t, typename ... Args>
        auto run_function_without_timeout (func_t &&func,
                Args &&... args) const -> function_result<typename std::result_of<func_t (Args...)>::type> {

          auto start = std::chrono::high_resolution_clock::now();
          auto result = func(std::forward<Args>(args)...);
          auto end = std::chrono::high_resolution_clock::now();
          return {result, std::chrono::duration_cast<std::chrono::microseconds>(end - start)};
        }

        template <typename func_t, typename ... Args>
        auto run_function (bool timeout,
                func_t &&func,
                Args &&... args) const -> function_result<typename std::result_of<func_t (Args...)>::type> {

          return (timeout ? run_function_with_timeout(func, std::forward<Args>(args)...)
                          : run_function_without_timeout(func, std::forward<Args>(args)...));
        }

        // =========================================================================

        static void terminate_process ();

        void update_stats (const function_status &,
                const std::chrono::microseconds & = std::chrono::microseconds::zero()) const;

        // =========================================================================

        template <typename First>
        static void _print_thread_safe (std::ostream &out, const First &value) {
          out << value;
          out.flush();
        }

        template <typename First, typename ... Rest>
        static void _print_thread_safe (std::ostream &out, const First &value, Rest &&... rest) {
          out << value;
          _print_thread_safe(out, std::forward<Rest>(rest)...);
        }

        template <typename ... Args>
        void print_thread_safe (const output_type &ot, Args &&... args) const {
          static std::mutex mtx;
          std::lock_guard<std::mutex> lock(mtx);
          _print_thread_safe(config.outputs.at(ot), std::forward<Args>(args)...);
        }

        // =========================================================================

        template <typename T>
        void print_value (const output_type &ot, const T &value) const {
          print_thread_safe(ot, value);
        }

        void print_value (const output_type &ot, const std::string &value) const;

        void print_value (const output_type &ot, const char value) const;

        template <typename T>
        void print_name_and_value (const output_type &ot, const std::string &name, const T &value) const {
          try {
            auto res = std::stold(name);
            print_thread_safe(ot, res);
          } catch (...) {
            if (config.print_names_of_arguments)
              print_thread_safe(ot, name, "=");

            print_value(ot, value);
          }
        }

        void print_name_and_value (const output_type &ot, const std::string &name, const std::string &value) const;

        void print_name_and_value (const output_type &ot, const std::string &name, const char value) const;

        template <typename First>
        void print_args (const output_type &ot,
                const std::vector<std::string> args_names,
                size_t pos,
                const First &first) const {

          print_name_and_value(ot, args_names.at(pos), first);
        }

        void print_args (const output_type &ot,
                const std::vector<std::string> args_names,
                size_t pos) const;

        template <typename First, typename ... Rest>
        void print_args (const output_type &ot,
                const std::vector<std::string> args_names,
                size_t pos,
                const First &first,
                Rest &&... rest) const {

          print_name_and_value(ot, args_names.at(pos), first);
          print_thread_safe(ot, ", ");
          print_args(ot, args_names, pos + 1, std::forward<Rest>(rest)...);
        }

        template <typename ... Args>
        void print_test_preamble (const output_type &ot,
                const std::string &func_name,
                const std::vector<std::string> &args_names,
                Args &&... args) const {

          if (suite_name == "") {
            print_thread_safe(ot, "running test ", func_name, "(");
          } else {
            print_thread_safe(ot, suite_name, ": running test ", func_name, "(");
          }
          print_args(ot, args_names, 0, std::forward<Args>(args)...);
          print_thread_safe(ot, ")");
        }

        template <typename ... Args>
        void print_assertion_preamble (const output_type &ot,
                const std::string &assertion_name) const {

          if (suite_name == "") {
            print_thread_safe(ot, "running assertion ", assertion_name);
          } else {
            print_thread_safe(ot, suite_name, ": running assertion ", assertion_name);
          }
        }
    };

    extern ptest_suite general_suite;
}

#include "suite_test_macro.hpp"
#include "test_macro.hpp"
#include "assert_macro.hpp"
#include "new_test_suite_macro.hpp"

#define print_final_test_result() {\
ptest::general_suite.print_general_result();\
}

#define print_final_suite_result(suite) {\
suite.print_suite_result();\
}

#endif //PLIB_PTEST_HPP
