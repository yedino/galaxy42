// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "gtest/gtest.h"
#include "fake_tuntap.hpp"

#include "../libs0.hpp"
#include "../cable/simulation/world.hpp"
#include "../galaxysrv.hpp"
#include "../c_crypto.hpp"
#include "../the_program_newloop.hpp"
#include "../netbuf.hpp"
#include "../cable/simulation/cable_simul_obj.hpp"
#include "../cable/simulation/cable_simul_addr.hpp"

TEST(the_program_new_loop, use_options_peerref) {
	g_dbg_level_set(200, "Be quiet in test (no fact output)");

	string argt_exe = "my_exec_name";
	vector<string> argt {"--peer", "192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5",
						 "--peer", "auto:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5",
						 "--peer", "udp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5",
						 "--peer", "tcp:192.166.218.58:19042-fd42:f6c4:9d19:f128:30df:b289:aef0:25f5",
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

	my_program.options_create_desc();
	my_program.options_parse_first();

	// can not be tested because of pimpl declaration i .cpp
	//my_program.pimpl->server.reset(std::make_unique<c_galaxysrv>());
	//my_program.use_options_peerref();
}

/*
TODO FIXME - fix c_cable_simul_obj (finish move to selector) @rfree
TEST(the_program_new_loop, fake_kernel) {
	c_tuntap_fake_kernel kernel;
	c_tuntap_fake tuntap_reader(kernel);

	c_crypto crypto;
	std::array<unsigned char, crypto_box_NONCEBYTES> nonce;

	auto world = make_shared<c_world>();

	unique_ptr<c_cable_base_obj> cable = make_unique<c_cable_simul_obj>( world );
	unique_ptr<c_cable_base_addr> peer_addr = make_unique<c_cable_simul_addr>( world->generate_simul_cable() );

	c_netbuf buf(200);
	_note("buf: " << make_report(buf,20) );

	if (false) {
		g_dbg_level_set(100, "Test data only");
		for(;;) {
			size_t read = tuntap_reader.readtun( reinterpret_cast<char*>( buf.data() ) , buf.size() );
			 c_netchunk chunk( buf.data() , read );
			_note("chunk: " << make_report(chunk,20) );
			 std::cout << to_debug( std::string(buf.data() , buf.data()+read) , e_debug_style_buf ) << std::endl;
		}
	} else {
		for(int cycle=0; cycle<10; ++cycle) {
			size_t read = tuntap_reader.readtun( reinterpret_cast<char*>( buf.data() ) , buf.size()-crypto_box_MACBYTES);
			c_netchunk chunk( buf.data() , read );
			_note("chunk: " << make_report(chunk,20) );
			_dbg3( to_debug( std::string(buf.data() , buf.data()+read) , e_debug_style_buf ) );
			crypto.cryptobox_encrypt(chunk.data(), read, nonce, crypto.get_my_public_key());
			// TODO incr nonce
			UsePtr(cable).send_to( UsePtr(peer_addr) , chunk.data() , chunk.size() );
		}
	}

	thread_test();
}
*/

