// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef include_formats_ip
#define include_formats_ip

#include <string>

typedef std::string t_ipv4dot; ///< a string that represents ipv4 in format "200.50.1.2"
typedef std::string t_ipv6dot; ///< a string of ipv6 address in format "fd42:ab:1:5::ccc:dddd" (dotted, allows compress-zeros)
typedef t_ipv6dot t_ipv46dot; ///< a string either in t_ipv4dot or in ipv6dots format (will be deduced on use from the format)

typedef std::string t_ipv4bin; ///< a string that represents ipv4 in binary format (e.g. 4 octets)
typedef std::string t_ipv6bin; ///< a string of ipv6 address in binary format (e.g. 128/8 = 16 octets)
typedef t_ipv6dot t_ipv46bin; ///< a string either in t_ipv4bin or t_ipv6bin format


#endif

