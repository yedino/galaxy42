#ifndef ASIO_IOSERVICE_MANAGER_HPP
#define ASIO_IOSERVICE_MANAGER_HPP

#include "libs0.hpp"
#include <boost/asio.hpp>

/// *SIOM* - Service_IO Manager
/// Manager for objects that will use ioservice, provides them with a pool of ioservice objects
/// References to #m_ioservice_array (and all it's elemets) remain valid as long as entire object of this class.
/// To avoid reallocations, the statically allocated array has compilation-time size #capacity()
/// While the run-time actuall number of running io_service is read by #size() and increased by #resize_to_at_least()
class c_asioservice_manager final {
	public:
		c_asioservice_manager(size_t size_); ///< start manager with size @param size
		~c_asioservice_manager();
		void resize_to_at_least(size_t size_); ///< set object size to at least this given size

		c_asioservice_manager(const c_asioservice_manager &)=delete; // not copyable
		c_asioservice_manager& operator=(const c_asioservice_manager &)=delete; // not copyable
		c_asioservice_manager(c_asioservice_manager &&)=delete; // not moveable
		c_asioservice_manager& operator=(c_asioservice_manager &&)=delete; // not moveable

		boost::asio::io_service &get_next_ioservice(); ///< returns reference to an ioservice, it is valid as long as this object.

		size_t capacity() const; ///< get capacity
		size_t size() const; ///< get current size

		void stop_all(); ///< runs stop on all our ioservices

	private:
		/// max supported count of ioservices that we can have (capacity). @see m_size for actuall
		/// We do not want to set in runtime to avoid list and to avoid reallocations of vectore.
		/// 1 object of ioservice seems to have totall runtime "weight" of <250 Byte on linux (measured in task bug#J441 by @mik)
		static constexpr size_t m_ioservice_max = 16;

		size_t m_size=0; ///< current actuall size, nubmer of existing ioservices
		size_t m_last_ioservice=0; ///< index of last-used ioservice, to know what to return next time
		std::array<boost::asio::io_service, m_ioservice_max> m_ioservice_array; ///< the ioservices
		std::vector<boost::asio::io_service::work> m_ioservice_idle_works;
		std::vector<std::thread> m_ioservice_threads;

		void run_ioservice(size_t index); ///< starts an existing, allocated service.
		void stop_ioservice(size_t index); ///< stops an existing, allocated service. Does NOT remove it! Just runs proper ASIO .stop()
};

#endif // ASIO_IOSERVICE_MANAGER_HPP
