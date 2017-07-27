
#include "utils/boost_asio_helper.hpp"
#include <tuple>

namespace detail {

/// Type TAsioAddr is needed to defer attempts to compile this function body untill after it is potentially-disabled
/// Type TAsioAddrBytes tells us does boost::asio use std::array or boost::array
template <typename TAsioAddr, typename TAsioAddrBytes>
boost::asio::ip::address_v6 make_ipv6_address_conversion( const std::array<unsigned char,16> & thebytes ,
	typename std::enable_if_t<std::is_same< TAsioAddrBytes, std::array<unsigned char,16> >::value>* = 0
	)
{
	TAsioAddr addr(thebytes); // asio wants std::array, and we have that, perfect
	return addr;
}


/// Type TAsioAddr is needed to defer attempts to compile this function body untill after it is potentially-disabled
/// Type TAsioAddrBytes tells us does boost::asio use std::array or boost::array
template <typename TAsioAddr, typename TAsioAddrBytes>
boost::asio::ip::address_v6 make_ipv6_address_conversion( const std::array<unsigned char,16> & thebytes ,
	typename std::enable_if_t<std::is_same< TAsioAddrBytes, boost::array<unsigned char,16> >::value>* = 0
	)
{
	constexpr auto thebytes_size = std::tuple_size<std::remove_reference_t<decltype(thebytes)>>::value ;
	boost::array<unsigned char, thebytes_size > thebytes_boost;
	for (int i=0; i<thebytes_size; ++i) thebytes_boost[i] = thebytes[i];
	boost::asio::ip::address_v6 addr(thebytes_boost);
	return addr;
}

} // namespace detail


boost::asio::ip::address_v6 make_ipv6_address(const std::array<unsigned char,16> & thebytes) {
	return detail::make_ipv6_address_conversion<
			boost::asio::ip::address_v6 , // address format we want
			boost::asio::ip::address_v6::bytes_type // byte format thatwe provide
		>( thebytes );
}

