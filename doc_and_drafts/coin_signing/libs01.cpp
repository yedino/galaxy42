#include "libs01.hpp"

namespace cs_utils {
    void print_strBytes(const std::string& str) {
        for(size_t i = 0; i < str.length(); ++i) {
            std::cout << static_cast<int>(str[i]) << ":";
       }
        std::cout << std::endl;
    }

    bool file_exsist (const std::string& filename) {
        std::ifstream f(filename.c_str());
        if (f.good()) {
            f.close();
            return true;
        } else {
            f.close();
          return false;
        }
    }

    // interesting :) should be standard in c++17
    //#include <experimental/filesystem>
    //namespace fs = std::experimental::filesystem;

    //void demo_exists(const fs::path& p, fs::file_status s = fs::file_status{})
    //{
    //    std::cout << p;
    //    if(fs::status_known(s) ? fs::exists(s) : fs::exists(p))
    //        std::cout << " exists\n";
    //    else
    //        std::cout << " does not exist\n";
    //}

    std::string generate_random_string (size_t length) {
        auto generate_random_char = [] () -> char {
            static const char Charset[] = "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(Charset) - 1);
            return Charset[rand() % max_index];
        };
        std::string str(length, 0);
        generate_n(str.begin(), length, generate_random_char);
        return str;
    }
    std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<uint64_t>> u64_to_time(uint64_t timestamp) {
        return std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>{std::chrono::duration<uint64_t>{timestamp}};
    }
}
