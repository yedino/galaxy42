

#include "vector_mutexed_obj.hpp"

const char * error_no_match_found::what() const noexcept { return "No matching object was found in container"; }

const char * error_not_enough_match_found::what() const noexcept { return "Not enough matching objects was found in container"; }


