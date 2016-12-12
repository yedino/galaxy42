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



#ifndef NetPlatform_H
#define NetPlatform_H

#include "syserr.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

extern const int Sockaddr_AF_INET;
extern const int Sockaddr_AF_INET6;

/// @return value <0 if an error occured.
t_syserr NetPlatform_addAddress(const char* interfaceName,
                            const uint8_t* address,
                            int prefixLen,
                            int addrFam);

/// @return value <0 if an error occured.
t_syserr NetPlatform_setMTU(const char* interfaceName,
                        uint32_t mtu);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
