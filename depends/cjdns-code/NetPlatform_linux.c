/*
 * Code from cjdns project
 * https://github.com/cjdelisle/cjdns
 *
 * commit 3b7802f00ab588edb3bb3d27f36b47f4b4524433
 * Author: rob <rob@mail.l>
 * Date:   Tue Feb 2 11:23:05 2016 +0000
 *
 *   [fix] default limit
 *
 *
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

 #ifdef __linux__
 
#include "NetPlatform.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/ipv6_route.h>
#include <stdint.h>

/**
 * This hack exists because linux/in.h and linux/in6.h define
 * the same structures, leading to redefinition errors.
 * For the second operand, we're grateful to android/bionic, platform level 21.
 */
#if !defined(_LINUX_IN6_H) && !defined(_UAPI_LINUX_IN6_H)
    struct in6_ifreq
    {
        struct in6_addr ifr6_addr;
        uint32_t ifr6_prefixlen;
        int ifr6_ifindex;
    };
#endif

const int Sockaddr_AF_INET = AF_INET;
const int Sockaddr_AF_INET6 = AF_INET6;

/**
 * Get a socket and ifRequest for a given interface by name.
 *
 * @param interfaceName the name of the interface, eg: tun0
 * @param af either AF_INET or AF_INET6
 * @param eg an exception handler in case something goes wrong.
 *           this will send a -1 for all errors.
 * @param ifRequestOut an ifreq which will be populated with the interface index of the interface.
 * @return a socket for interacting with this interface; Errors: -220 socket open,
 * -230 ioctl
 */
static int socketForIfName(const char* interfaceName,
						int af,
                           struct ifreq* ifRequestOut)
{
    int s;

    if ((s = socket(af, SOCK_DGRAM, 0)) < 0) {
    	return -220;
    }

    memset(ifRequestOut, 0, sizeof(struct ifreq));
    strncpy(ifRequestOut->ifr_name, interfaceName, IFNAMSIZ);

    if (ioctl(s, SIOCGIFINDEX, ifRequestOut) < 0) {
        close(s);
        return -230;
    }
    return s;
}

 /// @return 0=ok; Errors: -320 socket open, -330 ioctl
static int checkInterfaceUp(int socket,
                             struct ifreq* ifRequest)
{
    if (ioctl(socket, SIOCGIFFLAGS, ifRequest) < 0) {
        close(socket);
        return -320;
    }

    if (ifRequest->ifr_flags & IFF_UP & IFF_RUNNING) {
        // already up.
        return 0;
    }


    ifRequest->ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(socket, SIOCSIFFLAGS, ifRequest) < 0) {
        close(socket);
        return -330;
    }
    return 0;
}

/**
 * @return: 0=ok; Errors: -10 getaddrinfo, -20 socket open, -30 ioctl; -100 invalid addrFam -101 not implemented
   during socketForIfName:  -220 socket open, -230 ioctl,
   during checkInterfaceUp: -320 socket open, -330 ioctl
 */
int NetPlatform_addAddress(const char* interfaceName,
                            const uint8_t* address,
                            int prefixLen,
                            int addrFam)
{
    struct ifreq ifRequest;
    int s = socketForIfName(interfaceName, addrFam, &ifRequest);
    if (s < 0) return s;
    int ifIndex = ifRequest.ifr_ifindex;

    // checkInterfaceUp() clobbers the ifindex.
    int ifUp = checkInterfaceUp(s, &ifRequest);
    if (ifUp < 0) return ifUp;

    if (addrFam == Sockaddr_AF_INET6) {
        struct in6_ifreq ifr6;
        memset( &ifr6 , 0, sizeof(ifr6) ); // just to be sure
        ifr6.ifr6_prefixlen = prefixLen;
        ifr6.ifr6_ifindex = ifIndex;
        /* = {
            ifIndex,
            prefixLen
        };*/
        memcpy(&ifr6.ifr6_addr, address, 16);

        if (ioctl(s, SIOCSIFADDR, &ifr6) < 0) {
            close(s);
            return -30;
        }


    } else if (addrFam == Sockaddr_AF_INET) {
			printf("setting ipv4 address\n");
			printf("not implemented\n");
			return -101;
        /*struct sockaddr_in sin = { .sin_family = AF_INET, .sin_port = 0 };
        memcpy(&sin.sin_addr.s_addr, address, 4);
        memcpy(&ifRequest.ifr_addr, &sin, sizeof(struct sockaddr));

        if (ioctl(s, SIOCSIFADDR, &ifRequest) < 0) {
            close(s);
        }

        uint32_t x = ~0 << (32 - prefixLen);
        x = Endian_hostToBigEndian32(x);
        memcpy(&sin.sin_addr, &x, 4);
        memcpy(&ifRequest.ifr_addr, &sin, sizeof(struct sockaddr_in));

        if (ioctl(s, SIOCSIFNETMASK, &ifRequest) < 0) {
            close(s);
        }*/
    } else {
    	return -100;
    }

    close(s);
    return 0;
}

/**
 * @return 0=ok
 */
int NetPlatform_setMTU(const char* interfaceName,
                        uint32_t mtu)
{
    struct ifreq ifRequest;
    int s = socketForIfName(interfaceName, AF_INET6, &ifRequest);
    if (s<0) return s;

    ifRequest.ifr_mtu = mtu;
    if (ioctl(s, SIOCSIFMTU, &ifRequest) < 0) {
        close(s);
        return -30;
    }

    close(s);
    return 0;
}

#endif // __linux__
