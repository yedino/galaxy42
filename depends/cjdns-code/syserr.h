
#pragma once

typedef struct {
	int my_code; ///< my error code, depending on context
	int errno_copy; ///< copy of the errno with system error in it
} t_syserr;


