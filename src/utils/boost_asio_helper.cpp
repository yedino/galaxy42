
#include "utils/boost_asio_helper.hpp"
#include <tuple>

namespace detail {


/**
 * To normalize boost ASIO API, this will convert std::array into bytes_type used in your asio lib version
 * (that is, also to std::array or copy/convert to boost::array)
 */
boost::asio::ip::address_v6::bytes_type make_ipv6_address_bytes( const std::array<unsigned char,16> & bytes_std) {
	boost::asio::ip::address_v6::bytes_type bytes_asio;
	// static_assert( bytes_asio_size == std::tuple_size<decltype(bytes_std)>::value  , "The address size is invalid" );
	// can't test size of boost array in compile time. anyway, rather impossible to be invalid
	std::copy( bytes_std.cbegin(), bytes_std.cend(), bytes_asio.begin() );
	return bytes_asio;
}

} // namespace detail


boost::asio::ip::address_v6 make_ipv6_address(const std::array<unsigned char,16> & thebytes) {
	return boost::asio::ip::address_v6( detail::make_ipv6_address_bytes( thebytes ) );
}

