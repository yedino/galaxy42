#ifndef CRYPTO_RANDOM_GENERATOR_HPP
#define CRYPTO_RANDOM_GENERATOR_HPP

#include <string>
#include <fstream>
#include <stdexcept>

using std::string;
using std::ifstream;
using std::ios_base;

#ifdef TARGET_OS_MAC
        #define RANDOM_DEVICE "" // TODO

#elif defined __linux__
        #define RANDOM_DEVICE "/dev/urandom" // TODO change to /dev/random

#elif defined _WIN32 || defined _WIN64
        #define RANDOM_DEVICE "" // TODO

#elif defined __ANDROID_API__
        #define RANDOM_DEVICE "" // TODO

#endif


template <typename type>
class c_random_generator {
private:
        static std::ifstream m_reader;

public:
        c_random_generator () = default;
        ~c_random_generator () = default;

        static type get_random (size_t length_in_bytes) {
                if (!m_reader.is_open())
                        throw std::runtime_error("some error occured while reading from random number generator device");

                m_reader.rdbuf()->pubsetbuf(nullptr, 0); // stop buffering data
                type random = 0;
                unsigned char readed;
                size_t size_of_random = 0;
                while (size_of_random < length_in_bytes) {
                        readed = (unsigned char)m_reader.get();
                        random <<= (sizeof(readed) * 8);
                        random += (int)readed;
                        size_of_random += (sizeof(readed));
                }
                return random;
        }
};


template <typename type>
std::ifstream c_random_generator<type>::m_reader(RANDOM_DEVICE, ios_base::in);

#endif //CRYPTO_RANDOM_GENERATOR_HPP
