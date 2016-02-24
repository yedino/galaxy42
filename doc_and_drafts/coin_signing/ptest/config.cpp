#include <iostream>
#include <functional>
#include "config.hpp"

namespace ptest {
    size_t hash::operator() (const ptest::output_type &o) const {
      return std::hash<size_t>()(size_t(o));
    }

    config_t::config_t () { set_def_outputs(); }

    void config_t::set_def_outputs () {
      outputs = {
              {output_type::passed,    std::ref(std::cout)},
              {output_type::failed,    std::ref(std::cerr)},
              {output_type::timeout,   std::ref(std::cerr)},
              {output_type::exception, std::ref(std::cerr)},
              {output_type::info,      std::ref(std::cout)}
      };
    }

    void config_t::set_all_output_to (std::ostream &os) {
      outputs.at(output_type::passed)    = os;
      outputs.at(output_type::failed)    = os;
      outputs.at(output_type::timeout)   = os;
      outputs.at(output_type::exception) = os;
      outputs.at(output_type::info)      = os;
    }
}