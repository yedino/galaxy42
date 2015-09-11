#pragma once
#ifndef LIBS1_HPP
#define LIBS1_HPP

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <cmath>
#include <random>
#include <algorithm>
#include <exception>
#include <mutex>
#include <cassert>

//for libpng
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include <png.h>
#include <allegro.h>
#include "loadpng.hpp"

#include "c_tnetdbg.hpp"

using std::string;

using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;

using std::make_shared;

using std::vector;
using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::unordered_set;

using std::ostream;
using std::ifstream;
using std::istringstream;

#include "c_tnetdbg.hpp"

// http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
template <typename T, typename... Args>
std::unique_ptr<T> make_unique (Args &&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif

