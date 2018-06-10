// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#ifndef C_TUN_DEVICE_HPP
#define C_TUN_DEVICE_HPP

#include <array>
#include <string>
//TODO (da2ce7) CIRCULAR HEADER INCLUDE
#include "c_event_manager.hpp"
#include <chrono>

#include "platform.hpp"
#include "error_subtype.hpp"
#include "syserror_use.hpp"
#include <boost/asio.hpp>

/// @thread
std::string errno_to_string(int errno_copy); ///< Convert errno from C-lib into a string. Thread-safe function.

/**
 * @brief The c_tun_device class
 * Abstract class to represent tun device
 */
class c_tun_device {
	public:
		std::string m_ifr_name; ///< the name of interface, as it is needed by various ioctls. We try to set it in set_ipv6_address()
		bool m_ip6_ok; ///< did we yet succeed in setting IPv6

		c_tun_device();
		virtual ~c_tun_device() = default;
		virtual void init() = 0; ///< call before use

		virtual void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) = 0;
		virtual void set_mtu(uint32_t mtu) = 0; ///< sets MTU. first use set_ipv6_address
		virtual bool incomming_message_form_tun() = 0; ///< returns true if tun is readry for read
		virtual size_t read_from_tun(void *buf, size_t count) = 0;

                virtual size_t write_to_tun(void *buf, size_t count) = 0;

		virtual int get_tun_fd() const; ///< for POSIX-like systems, returns int fd of the tuntap file, e.g. for select()'ing it

    protected:
        typedef std::chrono::system_clock time;
        static const int number_of_tested_cards = 100;
        static const int cards_testing_time = 5;

};

#if defined(__FreeBSD__)
class c_tun_device_freebsd final : public c_tun_device {
	public:
    friend class c_event_manager_linux; // for io_service etc?

		c_tun_device_linux(){;}
		virtual void init(){} override; ///< call before use

		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override{};
		void set_mtu(uint32_t mtu) override;
		bool incomming_message_form_tun() override{return false;};
		size_t read_from_tun(void *buf, size_t count) override{return 0;}
		size_t write_to_tun(void *buf, size_t count) override{return 0};

		virtual int get_tun_fd() const override{return 0;};
};
#endif

#if defined(__OpenBSD__)
class c_tun_device_openbsd final : public c_tun_device {
        using stream_type = boost::asio::posix::stream_descriptor;
    public:
        friend class c_event_manager_openbsd; // for io_service etc?

        c_tun_device_openbsd();
        ~c_tun_device_openbsd();
        virtual void init(); ///< call before use

        void set_ipv6_address(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
        void set_mtu(uint32_t mtu) override;
        bool incomming_message_form_tun() override;
        size_t read_from_tun(void *buf, size_t count) override;
        size_t write_to_tun(void *buf, size_t count) override;

        virtual int get_tun_fd() const override;

        int m_tun_fd = -1;
        size_t m_readed_bytes;
        boost::asio::io_service m_ioservice;
        stream_type m_tun_stream;

        static inline int
        netbsd_modify_read_write_return(u_int32_t len) {
            if (len > 0) {
                return len > sizeof (u_int32_t) ? len - sizeof (u_int32_t) : 0;
            } else {
                return len;
            }
        }

        int
        write_tun(int tun0, void *buf, int len) {
            u_int32_t type;
            struct iovec iv[2];

            type = htonl(AF_INET6);

            iv[0].iov_base = (char *) &type;
            iv[0].iov_len = sizeof (type);
            iv[1].iov_base = buf;
            iv[1].iov_len = len;

            return netbsd_modify_read_write_return(writev(tun0, iv, 2));
        }

        int
        read_tun(int tun0, void *buf, int len) {
            u_int32_t type;
            struct iovec iv[2];

            iv[0].iov_base = (char *) &type;
            iv[0].iov_len = sizeof (type);
            iv[1].iov_base = buf;
            iv[1].iov_len = len;

            return netbsd_modify_read_write_return(readv(tun0, iv, 2));
        }

        int
        ipv6_mask(struct in6_addr *mask, int len) {
            // /netbsd-current/external/bsd/dhcpcd/dist/src/ipv6.c
            static const unsigned char masks[NBBY] = {0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff};
            int bytes, bits, i;

            if (len < 0 || len > 128) {
                errno = EINVAL;
                return -1;
            }

            memset(mask, 0, sizeof (*mask));
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
    };
#endif

#if defined(__linux__)
class c_tun_device_linux final : public c_tun_device {
	public:
    friend class c_event_manager_linux; // for io_service etc?

		c_tun_device_linux();
		virtual void init() override; ///< call before use

		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		void set_mtu(uint32_t mtu) override;
		bool incomming_message_form_tun() override;
		size_t read_from_tun(void *buf, size_t count) override;
		size_t write_to_tun(void *buf, size_t count) override;

		virtual int get_tun_fd() const override;

	private:
		int m_tun_fd; ///< the fd for TUN
};

// __linux__
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(__CYGWIN__)
	#ifndef __USE_W32_SOCKETS
		#define __USE_W32_SOCKETS
	#endif
#endif

#include "c_tun_device.hpp"
#include "c_ndp.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"
#include <array>
#include <boost/asio.hpp>
#include <ifdef.h>
#include <memory>
#include <windows.h>
#include <vector>

class c_tun_device_windows final : public c_tun_device, c_is_user_admin {
	//friend class c_ndp;
public:
  friend class c_event_manager_asio; // for io_service etc?

	c_tun_device_windows();
	virtual void init() override; ///< call before use

	void set_ipv6_address
		(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
	void set_mtu(uint32_t mtu) override {}; // TODO
	bool incomming_message_form_tun() override; ///< returns true if tun is ready for read
	size_t read_from_tun(void *buf, size_t count) override; ///< count must be <= size of buffer in buf! otherwise UB
        size_t write_to_tun(void *buf, size_t count) override; ///< count must be <= the size of buffer in buf! otherwise UB

																 //constexpr int m_buffer_size = 9000; ///< the buffer size. This can affect size of MTU that this TUN can offer maybe

private:
	std::wstring m_guid;
	boost::asio::io_service m_ioservice;
	std::array<uint8_t, 9000> m_buffer;
	size_t m_readed_bytes; ///< currently read bytes that await in m_buffer
	HANDLE m_handle; ///< windows handle to the TUN device
	std::unique_ptr<boost::asio::windows::stream_handle> m_stream_handle_ptr; ///< boost handler to the TUN device
	std::array<uint8_t, 6> m_mac_address;

	std::vector<std::wstring> get_subkeys(HKEY hKey); ///< for windows registry
	std::wstring get_device_guid(); ///< technical name of the device
	std::wstring get_human_name(const std::wstring &guid);
	NET_LUID get_luid(const std::wstring &human_name);
	HANDLE get_device_handle();
	HANDLE open_tun_device(const std::wstring &guid); // returns opened handle for guid or INVALID_HANDLE_VALUE
	std::array<uint8_t, 6> get_mac(HANDLE handle); // get handle to opened device (returned by get_device_handle())

	void handle_read(const boost::system::error_code& error, std::size_t length); ///< ASIO handler
	class hkey_wrapper final {
		public:
			/**
			 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
			 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
			 */
			hkey_wrapper(HKEY hkey);
			~hkey_wrapper();
			HKEY &get();
			/**
			 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
			 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
			 */
			void set(HKEY new_hkey);
			void close();
		private:
			HKEY m_hkey;
			bool m_is_open;
	};
};

// _win32 || __cygwin__
#endif

#if defined(__MACH__)
#include <boost/asio.hpp>
class c_tun_device_apple final : public c_tun_device {
public:
  friend class c_event_manager_asio; // for io_service etc?

    c_tun_device_apple();
		virtual void init() override; ///< call before use
    void set_ipv6_address
            (const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
    void set_mtu(uint32_t mtu) override;
    bool incomming_message_form_tun() override; ///< returns true if tun is readry for read
    size_t read_from_tun(void *buf, size_t count) override;
    size_t write_to_tun(void *buf, size_t count) override;
private:
    int m_tun_fd;
    boost::asio::io_service m_ioservice;
    std::unique_ptr<boost::asio::posix::stream_descriptor> m_stream_handle_ptr; ///< boost handler to the TUN device
    std::array<uint8_t, 9000> m_buffer;
    size_t m_readed_bytes; ///< currently read bytes that await in m_buffer

		virtual int get_tun_fd() const override;
		int create_tun_fd();
    void handle_read(const boost::system::error_code &error, size_t length);
};
#endif

#if defined(EMPTY)
#warning "using c_tun_device_empty = It can not work!"
class c_tun_device_empty final : public c_tun_device {
	public:
		c_tun_device_empty();
		void init(); ///< call before use
		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		void set_mtu(uint32_t mtu) override;
		bool incomming_message_form_tun() override;
		size_t read_from_tun(void *buf, size_t count) override;
		size_t write_to_tun(const void *buf, size_t count) override;
};
#endif

#endif // C_TUN_DEVICE_HPP
