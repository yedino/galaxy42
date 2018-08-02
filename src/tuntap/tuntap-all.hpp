#pragma once

#include <tuntap/fake_tun/c_fake_tun.hpp>

#if defined(ANTINET_linux)
#include <tuntap/linux/c_tuntap_linux_obj.hpp>
#endif
#if defined(ANTINET_windows)
#include <tuntap/windows/c_tuntap_windows.hpp>
#endif
#if defined(ANTINET_macosx)
#include <tuntap/macosx/c_tuntap_macosx_obj.hpp>
#endif
#if defined(ANTINET_netbsd)
#include <tuntap/netbsd/c_tuntap_netbsd_obj.hpp>
#endif
#if defined(ANTINET_openbsd)
#include <tuntap/openbsd/c_tuntap_openbsd_obj.hpp>
#endif
#if defined(ANTINET_freebsd)
#include <tuntap/freebsd/c_tuntap_freebsd_obj.hpp>
#endif