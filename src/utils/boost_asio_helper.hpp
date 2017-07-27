
#pragma once

#include <array>
#include <boost/asio.hpp>

/**
  * Make address from std::array. This is needed because boost can use std::array or boost::array so this
  * detects what boost asio needs and converts. (e.g. Boost version in Debian 8 uses other option then newer versions).
  */
boost::asio::ip::address_v6 make_ipv6_address(const std::array<unsigned char,16> & thebytes);

