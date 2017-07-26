
#pragma once

#include <array>
#include <boost/asio.hpp>

/**
  * Make address from std::array, as this conversion would not work on older versions od Boost (e.g. Debian 8 default)
  */
boost::asio::ip::address_v6 make_address( std::array<unsigned char,16> thebytes);

