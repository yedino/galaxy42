#ifndef ASIO_IOSERVICE_MANAGER_HPP
#define ASIO_IOSERVICE_MANAGER_HPP

#include "libs0.hpp"
#include <boost/asio.hpp>

/**
@brief Manager for objects that will use ioservice, provides them with a pool of ioservice objects;
References to #m_ioservice_array (and all it's elemets) remain valid as long as entire object of this class.
To avoid reallocations, the statically allocated array has compilation-time size #capacity()
While the run-time actuall number of running io_service is read by #size() and increased by #resize_to_at_least()

@thread thread_safe_public_myself

@warning For class developers:
all resizing must be done in one place, in #resize_to_at_least() !
#m_ioservice_array - has static capacity and is "resized" by us by #m_size
#m_ioservice_idle_works + #m_ioservice_threads - is resized by us

@warning All public operations must happen under mutex - #m_mutex
@owner rob
*/
class c_asioservice_manager final {
	public:
		c_asioservice_manager(size_t size_); ///< start manager with size @param size_
		~c_asioservice_manager();

		/// [thread_safe] set object size to at least this given size. After this all internal arrays for threads
		/// of number 0 .. size_-1 have created elements ready to be used
		void resize_to_at_least(size_t size_);

		boost::asio::io_service &get_next_ioservice(); ///< [thread_safe] returns reference to an ioservice, it is valid as long as this object.

		size_t capacity() const; ///< [thread_safe] get capacity
		size_t size() const; ///< [thread_safe] get current size

		void stop_all_threadsafe(); ///< [thread_safe] runs stop on all our ioservices

		c_asioservice_manager(const c_asioservice_manager &)=delete; // not copyable
		c_asioservice_manager& operator=(const c_asioservice_manager &)=delete; // not copyable
		c_asioservice_manager(c_asioservice_manager &&)=delete; // not moveable
		c_asioservice_manager& operator=(c_asioservice_manager &&)=delete; // not moveable

	public: // TODO protected
		void Precond() const;
		void Postcond() const;
		friend class c_ig_friendly;
		friend class c_ig< c_asioservice_manager >;

	private:
		/// max supported count of ioservices that we can have (capacity). @see m_size for actuall
		/// We do not want to set in runtime to avoid list and to avoid reallocations of vectore.
		/// 1 object of ioservice seems to have totall runtime "weight" of <250 Byte on linux (measured in task bug#J441 by @mik)
		static constexpr size_t m_ioservice_max = 16;

		size_t m_size=0; ///< current actuall size, nubmer of existing ioservices
		size_t m_last_ioservice=0; ///< index of last-used ioservice, to know what to return next time
		std::array<boost::asio::io_service, m_ioservice_max> m_ioservice_array; ///< the ioservices

		std::vector<boost::asio::io_service::work> m_ioservice_idle_works; ///< IDLE work so io_service.run will not exit
		std::vector<std::thread> m_ioservice_threads; ///< the thread of work for #m_ioservice_idle_works

		/// starts an existing, allocated service (caller made proper resize).
		/// @pre Object must be already resized to proper size (so that index is valid, size is at least index+1)
		void run_ioservice(size_t index);

		void stop_ioservice(size_t index); ///< stops an existing, allocated service (does NOT remove/resize)

		mutable std::recursive_mutex m_mutex; ///< mutex for self-protection to implement certain thread_safe functions,
		/// it is recursive so that from our self-locking public functions we can call also our other public functions, e.g. #size()

		/// if set to yes then I should stop all my jobs: e.g. will refuse creating new ones etc,
		/// this must be set e.g. to not add more started jobs after I decided to stop
		bool m_stop;
};

#endif // ASIO_IOSERVICE_MANAGER_HPP
