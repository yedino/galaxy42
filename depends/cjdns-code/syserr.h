
#pragma once

typedef struct {
	int my_code; ///< if <=0 then this is my error code (depending on context), else this is special value (e.g. int fd)
	int errno_copy; ///< copy of the errno with system error in it if my_code<0, otherwise ignore it (but should be 0)
} t_syserr;

typedef enum {
    e_netplatform_err_getaddrinfo=-10,
    e_netplatform_err_open_socket=-20,
	e_netplatform_err_open_fd=-25,
	e_netplatform_err_ioctl=-30,
    e_netplatform_err_invalid_addr_family=-100,
    e_netplatform_err_not_impl_addr_family=-101,
    e_netplatform_err_socketForIfName_open=-220,
    e_netplatform_err_socketForIfName_ioctl=-230,
    e_netplatform_err_checkInterfaceUp_open=-320,
    e_netplatform_err_checkInterfaceUp_ioctl=-330

/// @return .my_code: 0=ok; Errors: -10 getaddrinfo, -20 socket open, -30 ioctl; -100 invalid addrFam
} t_netplatform_err;


