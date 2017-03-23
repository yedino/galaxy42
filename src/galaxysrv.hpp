
// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "galaxysrv.hpp"
#include "galaxysrv_peers.hpp"
#include "galaxysrv_cables.hpp"
#include "galaxysrv_p2p.hpp"
#include "galaxysrv_e2e.hpp"

#include "libs1.hpp"

#include <boost/program_options.hpp>

#include "protocol.hpp"
#include "c_peering.hpp"
#include "generate_crypto.hpp"

#include "crypto/crypto.hpp" // for tests
#include "rpc/rpc.hpp"

#include "trivialserialize.hpp"
#include "galaxy_debug.hpp"

#include "glue_sodiumpp_crypto.hpp" // e.g. show_nice_nonce()

#include "ui.hpp"

#include "tunserver.hpp"
#include "c_tun_device.hpp"
#include "c_udp_wrapper.hpp"
#include "c_event_manager.hpp"
#include <ctime>
#ifdef HTTP_DBG
#include "httpdbg/httpdbg-server.hpp"
#endif

#include "tuntap/base/tuntap_base.hpp"
#include "tuntap/linux/c_tuntap_linux_obj.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"
#include "tuntap/macosx/c_tuntap_macosx_obj.hpp"

#include "platform.hpp"

// gal.peers.peers_add();

/// @owner supernatural_net_owner this is very powerfull owner be aware of making him mad.
class c_galaxysrv : public c_galaxysrv_peers, c_galaxysrv_cables, c_galaxysrv_p2p, c_galaxysrv_e2e {
	public:
		c_galaxysrv()=default;
		virtual ~c_galaxysrv()=default;

		/// load my (this node's) keypair from it's default store (e.g. config files)
		/// if your files seem to not be yet created, then throws expected_not_found
		void configure_mykey();

		/// @name Functions that execute a program action like creation of key, calculating signature, etc.
		/// @{
		void program_action_set_IDI(const string & keyname); ///< set configured IDI key (write the config to disk)
		void program_action_gen_key(const boost::program_options::variables_map & argm); ///< generate a key according to given options
		std::string program_action_gen_key_simple(); ///< generates recommended simple key, returns name e.g. "IDI"
		/// @}

		void init_tuntap();

		c_haship_addr get_my_hip() const; ///< get your main hash-ip (ipv6)

		void main_loop();

		void start_exit(); ///< start exiting: sets #m_exiting and tries to close connections etc.

	protected:

		/// @name My crypto identity
		/// @{

		/// priv+pub keypair for my Current identity (IDC)
		antinet_crypto::c_multikeys_PAIR m_my_IDC;
		/// IDI - the Installation Identnity, as only public-key, to remove privte from ram for protection
		antinet_crypto::c_multikeys_pub	m_my_IDI_pub;
		/// 'signature' - msg=IDC_pub, signer=IDI - it proves that IDI authorized this IDC
		antinet_crypto::c_multisign m_IDI_IDC_sig;
		c_haship_addr m_my_hip; ///< my HIP that results from m_my_IDC, already cached in this format

		/// @}

		/// @name tuntap access
		/// @{
		#ifdef ANTINET_linux
			c_tuntap_linux_obj m_tuntap;
		#elif defined(ANTINET_windows)
			c_tuntap_windows_obj m_tuntap;
		#elif defined(ANTINET_macosx)
			c_tuntap_macosx_obj m_tuntap;
		#else
			#error "This platform is not supported"
		#endif
		/// @}

		bool m_exiting=false; ///< are we now in exiting mode, then we should be refusing/closing connections etc
};

// -------------------------------------------------------------------
