#ifndef LIBS_H
#define LIBS_H

#define UNUSED(expr) do { (void)(expr); } while (0)
#define DBG_MTX(X,Y) do{X.lock();std::cout<< __func__<<":\t\t "<<Y<<std::endl;X.unlock();}while(0)

#include <exception>
#include <stdexcept>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <list>
#include <vector>

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <ctime>

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <cmath>
#include <algorithm>


#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/binary_object.hpp>

#include "../../crypto_ops/crypto/crypto_ed25519.hpp"
#include "../../crypto_ops/crypto/c_random_generator.hpp"

using std::size_t;

void print_strBytes(const std::string& str);
bool file_exsist (const std::string& filename);

#endif // LIBS_H
