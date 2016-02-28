#include "libs01.hpp"

void print_strBytes(const std::string& str) {
    for(size_t i = 0; i < str.length(); ++i) {
        std::cout << static_cast<int>(str[i]) << ":";
    }
    std::cout << std::endl;
}
