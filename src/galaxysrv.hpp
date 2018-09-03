
// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

/// @owner owner_of_file

#include "galaxysrv_peers.hpp"
#include "galaxysrv_cables.hpp"
#include "galaxysrv_p2p.hpp"
#include "galaxysrv_e2e.hpp"
#include "galaxysrv_engine.hpp"

#include <boost/program_options.hpp>

#include <ctime>
#ifdef HTTP_DBG
#include "httpdbg/httpdbg-server.hpp"
#endif

#include "tuntap/linux/c_tuntap_linux_obj.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"
#include "tuntap/macosx/c_tuntap_macosx_obj.hpp"


/// @owner magaNet_user
class c_galaxysrv : public c_galaxysrv_peers, c_galaxysrv_cables, c_galaxysrv_p2p, c_galaxysrv_e2e, c_galaxysrv_engine {
	public:
		c_galaxysrv()=default;
		virtual ~c_galaxysrv()=default;

		/// load my (this node's) keypair from it's default store (e.g. config files)
		/// if your files seem to not be yet created, then throws expected_not_found
		void configure_mykey(const std::string &ipv6_prefix);

		/// @name Functions that execute a program action like creation of key, calculating signature, etc.
		/// @{
		void program_action_set_IDI(const std::string & keyname); ///< set configured IDI key (write the config to disk)
		void program_action_gen_key(const boost::program_options::variables_map & argm); ///< generate a key according to given options
		std::string program_action_gen_key_simple(); ///< generates recommended simple key, returns name e.g. "IDI"
		/// @}

		void set_prefix_len(int prefix_len); ///< set prefix len, must be called before init_tuntap()
		void init_tuntap();

		c_haship_addr get_my_hip() const; ///< get your main hash-ip (ipv6)

		/// @owner main_loop_engineer refactored fast loop that do everything
		void main_loop();

		void start_exit(); ///< start exiting: sets #m_exiting and tries to close connections etc.

		uint16_t get_tuntap_mtu_default() const; ///< get the tuntap MTU size that we want to have by default, e.g. on startup
		uint16_t get_tuntap_mtu_current() const; ///< get the tuntap MTU size that we have now

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
		std::array<unsigned char, 2> m_my_hip_prefix; ///< first 16 bits of ipv6 address, i.e. fd42

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
		int m_prefix_len = -1; ///< tun ipv6 address prefix length
};

// -------------------------------------------------------------------
