/* vim: set expandtab ts=4 sw=4: */
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __MACH__

#include "Endian.h"
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <string.h>
#include <netdb.h>
#include <net/if_var.h>
#include <netinet/in_var.h>
#include <netinet6/nd6.h>
#include <netinet/in.h>
#include <sys/kern_control.h>
#include <sys/sys_domain.h>
#include <sys/kern_event.h>
#include <net/route.h>
#include <sys/sysctl.h>

#include "syserr.h"

#define ArrayList_TYPE struct Sockaddr
#define ArrayList_NAME OfSockaddr

/** Returned by Hex_decode() or Hex_encode() if the output buffer is too small. */
#define Hex_TOO_BIG -2

const int Sockaddr_AF_INET = AF_INET;
const int Sockaddr_AF_INET6 = AF_INET6;

static const char* hexEntities = "0123456789abcdef";

static int Hex_encode(uint8_t* output,
               const uint32_t outputLength,
               const uint8_t* in,
               const uint32_t inputLength)
{
    if (outputLength < inputLength * 2) {
        return Hex_TOO_BIG;
    } else if (outputLength > inputLength * 2) {
        output[inputLength * 2] = '\0';
    }

    for (uint32_t i = 0; i < inputLength; i++) {
        output[i * 2] = hexEntities[in[i] >> 4];
        output[i * 2 + 1] = hexEntities[in[i] & 15];
    }

    return inputLength * 2;
}

static void AddrTools_printIp(uint8_t output[40], const uint8_t binIp[16])
{
    uint8_t hex[32];
    Hex_encode(hex, 32, binIp, 16);

    output[ 0] = hex[ 0];
    output[ 1] = hex[ 1];
    output[ 2] = hex[ 2];
    output[ 3] = hex[ 3];
    output[ 4] = ':';
    output[ 5] = hex[ 4];
    output[ 6] = hex[ 5];
    output[ 7] = hex[ 6];
    output[ 8] = hex[ 7];
    output[ 9] = ':';
    output[10] = hex[ 8];
    output[11] = hex[ 9];
    output[12] = hex[10];
    output[13] = hex[11];
    output[14] = ':';
    output[15] = hex[12];
    output[16] = hex[13];
    output[17] = hex[14];
    output[18] = hex[15];
    output[19] = ':';
    output[20] = hex[16];
    output[21] = hex[17];
    output[22] = hex[18];
    output[23] = hex[19];
    output[24] = ':';
    output[25] = hex[20];
    output[26] = hex[21];
    output[27] = hex[22];
    output[28] = hex[23];
    output[29] = ':';
    output[30] = hex[24];
    output[31] = hex[25];
    output[32] = hex[26];
    output[33] = hex[27];
    output[34] = ':';
    output[35] = hex[28];
    output[36] = hex[29];
    output[37] = hex[30];
    output[38] = hex[31];
    output[39] = '\0';
}

/// @return .my_code: 0=ok; Errors: -20 socket open, -30 ioctl
static t_syserr addIp4Address(const char* interfaceName,
                          const uint8_t address[4],
                          int prefixLen)
{
    struct ifaliasreq ifarted;
    memset(&ifarted, 0, sizeof(struct ifaliasreq));
    strncpy(ifarted.ifra_name, interfaceName, IFNAMSIZ);

    struct sockaddr_in sin = { .sin_family = AF_INET, .sin_len = sizeof(struct sockaddr_in) };
    memcpy(&sin.sin_addr.s_addr, address, 4);
    memcpy(&ifarted.ifra_addr, &sin, sizeof(struct sockaddr_in));
    sin.sin_addr.s_addr = Endian_hostToBigEndian32(~0 << (32 - prefixLen));
    memcpy(&ifarted.ifra_mask, &sin, sizeof(struct sockaddr_in));

    int s = socket(AF_INET, SOCK_DGRAM, 0); // can set errno
    if (s < 0) return { -20 , errno };

    // will probably fail, ignore result.
    struct ifreq ifr = { .ifr_flags = 0 };
    strncpy(ifr.ifr_name, interfaceName, IFNAMSIZ);
    ioctl(s, SIOCDIFADDR, &ifr); // can set errno
    // ignoring this error on purpose.

    if (ioctl(s, SIOCSIFADDR, &ifarted) < 0) { // can set errno
        int err = errno;
        close(s);
        return { -30 , err }; // return saved copy of errno
    }

    //setupRoute4(address, prefixLen, interfaceName, logger, tempAlloc, eh);

    close(s);
    return {0,0};
}

/// @return .my_code: 0=ok; Errors: -10 getaddrinfo, -20 socket open, -30 ioctl
static t_syserr addIp6Address(const char* interfaceName,
                          const uint8_t address[16],
                          int prefixLen)
{
    /* stringify our IP address */
    char myIp[40];
    AddrTools_printIp((uint8_t*)myIp, address);

    /* set up the interface ip assignment request */
    struct in6_aliasreq in6_addreq;
    memset(&in6_addreq, 0, sizeof(in6_addreq));
    in6_addreq.ifra_lifetime.ia6t_vltime = ND6_INFINITE_LIFETIME;
    in6_addreq.ifra_lifetime.ia6t_pltime = ND6_INFINITE_LIFETIME;

    /* parse the IPv6 address and add it to the request */
    struct addrinfo hints, *result;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    int err = getaddrinfo((const char *)myIp, NULL, &hints, &result); // can set errno
    if (err) return { -10 , errno };

    bcopy(result->ai_addr, &in6_addreq.ifra_addr, result->ai_addrlen);

    /* turn the prefixlen into a mask, and add it to the request */
    struct sockaddr_in6* mask = &in6_addreq.ifra_prefixmask;

    mask->sin6_len = sizeof(*mask);
    if (prefixLen >= 128 || prefixLen <= 0) {
        memset(&mask->sin6_addr, 0xff, sizeof(struct in6_addr));
    } else {
        memset((void *)&mask->sin6_addr, 0x00, sizeof(mask->sin6_addr));
        memset((void *)&mask->sin6_addr, 0xff, prefixLen>>3);
        ((uint8_t*)&mask->sin6_addr)[prefixLen>>3] = 0xff << (8 - (prefixLen%8));
    }

    strncpy(in6_addreq.ifra_name, interfaceName, sizeof(in6_addreq.ifra_name));

    /* do the actual assignment ioctl */
    int s = socket(AF_INET6, SOCK_DGRAM, 0); // can set errno
    if (s < 0) return { -20 , errno };

    if (ioctl(s, SIOCAIFADDR_IN6, &in6_addreq) < 0) { // can set errno
        int err = errno;
        close(s);
        return { -30 , err };
    }

    close(s);
    return {0,0};
}

/// @return: .my_code is: 0=ok, else errors: -10 getaddrinfo, -20 socket open, -30 ioctl; -100 invalid addrFam
t_syserr NetPlatform_addAddress(const char* interfaceName,
                            const uint8_t* address,
                            int prefixLen,
                            int addrFam)
{
    if (addrFam == Sockaddr_AF_INET6) {
        return addIp6Address(interfaceName, address, prefixLen);
    } else if (addrFam == Sockaddr_AF_INET) {
        return addIp4Address(interfaceName, address, prefixLen);
    } else {
    	return { -100 , 0 } ;
    }
    return {0,0};
}

/// @return .my_code: 0=ok; Errors: -10 getaddrinfo, -20 socket open, -30 ioctl; -100 invalid addrFam
t_syserr NetPlatform_setMTU(const char* interfaceName,
                        uint32_t mtu)
{
    int s = socket(AF_INET6, SOCK_DGRAM, 0); // can set errno
    if (s < 0) return { -20 , errno };

    struct ifreq ifRequest;
    strncpy(ifRequest.ifr_name, interfaceName, IFNAMSIZ);
    ifRequest.ifr_mtu = mtu;
    if (ioctl(s, SIOCSIFMTU, &ifRequest) < 0) { // can set errno
       int err = errno;
       close(s);
       return { -30 , err }; // the saved errno is returned too
    }
    close(s); // close file, it was opened since socket() succeeded.
    return {0,0};
}


#endif // __MACH__
