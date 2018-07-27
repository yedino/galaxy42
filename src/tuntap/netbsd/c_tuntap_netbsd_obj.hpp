#ifndef C_TUNTAP_NETBSD_OBJ_HPP
#define C_TUNTAP_NETBSD_OBJ_HPP

#if defined(ANTINET_netbsd)
#include "platform.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>

#include <sys/socket.h>
#include <net/if_tun.h>
#include <netinet6/in6_var.h>
#include <netinet6/nd6.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "c_tun_device.hpp"

#include "../base/tuntap_base.hpp"
#include "../../test/mock_posix_stream_descriptor.hpp"
#include "../../test/mock_tuntap_system_functions.hpp"
#include "../../i_tuntap_system_functions.hpp"

class c_tuntap_system_functions final : public i_tuntap_system_functions {
    public:
        c_tuntap_system_functions() = default;
        ~c_tuntap_system_functions() = default;
        
        int ioctl(int fd, unsigned long request,  void *ifreq) override 
        { 
            pfp_UNUSED(fd);
            pfp_UNUSED(request);
            pfp_UNUSED(ifreq);
            return 0; 
        };
        
        t_syserr NetPlatform_addAddress(const char* interfaceName,
                                        const uint8_t* address,
                                        int prefixLen,
                                        int addrFam) override 
        { 
            pfp_UNUSED(interfaceName);
            pfp_UNUSED(address);
            pfp_UNUSED(prefixLen);
            pfp_UNUSED(addrFam);
            return {0,0}; 
        };
                                        
        t_syserr NetPlatform_setMTU(const char* interfaceName,
                                    uint32_t mtu) override 
        { 
            pfp_UNUSED(interfaceName);
            pfp_UNUSED(mtu);
            return {0,0}; 
        };
};

class c_tuntap_netbsd_obj final : public c_tuntap_base_obj {
    using sys_functions_wrapper = c_tuntap_system_functions;
    using stream_type = boost::asio::posix::stream_descriptor;
    
    public:
        c_tuntap_netbsd_obj(); ///< construct this object, throws if error
        c_tuntap_netbsd_obj(boost::asio::io_service &io_service);
        ~c_tuntap_netbsd_obj();

        size_t send_to_tun(
            const unsigned char *data, 
            size_t size) 
        override;
        
        size_t send_to_tun_separated_addresses(
            const unsigned char * const data,
            size_t size,
            const std::array<unsigned char, IPV6_LEN> &src_binary_address,
            const std::array<unsigned char, IPV6_LEN> &dst_binary_address);
        
        size_t read_from_tun(
            unsigned char * const data, 
            size_t size) 
        override;
        
        size_t read_from_tun_separated_addresses(
            unsigned char * const data, 
            size_t size,
            std::array<unsigned char, IPV6_LEN> &src_binary_address,
            std::array<unsigned char, IPV6_LEN> &dst_binary_address) 
        override;
        
        void async_receive_from_tun(
            unsigned char * const data, 
            size_t size, 
            const read_handler & handler) 
        override;
        
        void set_tun_parameters(
            const std::array<unsigned char, IPV6_LEN> &binary_address, 
            int prefix_len, 
            uint32_t mtu);

        stream_type &get_native_asio_object() {
            return m_tun_stream;
        }
        
	private:
            // start: from openvpn tun
            static inline int
            netbsd_modify_read_write_return(u_int32_t len) {
                if (len > 0) {
                    return len > sizeof(u_int32_t) ? len - sizeof(u_int32_t) : 0;
                } else {
                    return len;
                }
            }

            int
            write_tun(int tun0, const unsigned char *buf, int len)
            {
                u_int32_t type;
                struct iovec iv[2];

                type = htonl(AF_INET6);

                iv[0].iov_base = (char *)&type;
                iv[0].iov_len = sizeof(type);
                iv[1].iov_base = (void *)buf;
                iv[1].iov_len = len;

                return netbsd_modify_read_write_return(writev(tun0, iv, 2));
            }

            int
            read_tun(int tun0, const unsigned char *buf, int len)
            {
                u_int32_t type;
                struct iovec iv[2];

                iv[0].iov_base = (char *)&type;
                iv[0].iov_len = sizeof(type);
                iv[1].iov_base = (void *)buf;
                iv[1].iov_len = len;

                return netbsd_modify_read_write_return(readv(tun0, iv, 2));
            }
            // end: from openvpn tun
            
            int ipv6_mask(struct in6_addr *mask, int len) {
                // /netbsd-current/external/bsd/dhcpcd/dist/src/ipv6.c
                static const unsigned char masks[NBBY] = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };
                int bytes, bits, i;

                if (len < 0 || len > 128) {
                    errno = EINVAL;
                    return -1;
                }

                memset(mask, 0, sizeof(*mask));
                bytes = len / NBBY;
                bits = len % NBBY;
               for (i = 0; i < bytes; i++)
                    mask->s6_addr[i] = 0xff;
                if (bits) {
                    /* Coverify false positive.
                    * bytelen cannot be 16 if bitlen is non zero */
                    /* coverity[overrun-local] */
                    mask->s6_addr[bytes] = masks[bits - 1];
                }
                return 0;
            }
            
            const int                           m_tun_fd;
            boost::asio::io_service             m_io_service;
            stream_type                         m_tun_stream;
            sys_functions_wrapper               sys_fun;
            c_tun_device_bsd                    tundn;
};
#endif

#endif /* C_TUNTAP_NETBSD_OBJ_HPP */
