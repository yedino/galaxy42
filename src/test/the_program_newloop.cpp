// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "../the_program_newloop.hpp"

TEST(the_program_new_loop, use_options_peerref) {
	string argt_exe = "my_exec_name";
	vector<string> argt {"--peer", "192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5",
						 "--peer", "auto:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"
						 "--peer", "udp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"
						 "--peer", "tcp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"
						 "--peer", "shm:test1-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5"};
	/*
	"--peer 192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer auto:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer udp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer tcp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	"--peer shm:test1-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5 [NOT_IMPLEMENTED]"
	*/
	c_the_program_newloop my_program;
	my_program.take_args(argt_exe , argt);
}
