#include "additionals.hpp"

namespace ptest {
    const char *timeout_exception::what () const throw() {
      return "Timeout reached";
    }
}