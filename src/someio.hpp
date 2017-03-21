

#pragma once

#include "libs0.hpp"

#include <boost/asio.hpp>

/**

Class that provides some I/O operations, that can have stats, and thread-controll e.g. are cancellable.

@section unblock
We need a way to avoid infinite waiting for both blocking and async operations;
This is done by providing thread-safe method unblock_and_stop().
It will provide a way to unblock eventually the blocking-reads (e.g. by heaving socket option of timeout)
and to unblock async reads (e.g. by calling io_service.stop() for ASIO).

*/
class c_someio {
	protected:
		c_someio(); ///< to be used by child classes (and factories)

	public:
		virtual ~c_someio()=default;

		virtual void stop_threadsafe(); ///< will *soon* stop all *blocking* and *async* operations.

		std::chrono::microseconds sockopt_timeout_get_default() const; ///< returns the default timeout, see #sockopt_timeout()

		using t_native_socket = boost::asio::ip::udp::socket::native_handle_type; ///< this platforms handler for native objects
		static_assert(
			std::is_same<
				boost::asio::ip::udp::socket::native_handle_type ,
				boost::asio::ip::tcp::socket::native_handle_type >
				::value
			, "Hmm can't find same native handler for e.g. UDP sockets and TCP sockets? Then what type to pass to set_sockopt_timeout here?"
		);

	protected:
		std::atomic<bool> m_stop; ///< should we stop our operations, see #unblock

		/// on some native socket given by caller, sets the timeout, used for blocking read - latency of exit flag #m_stop
		/// call this from child classes on all sockets (e.g. asio sockets); see #unblock
		void set_sockopt_timeout(t_native_socket sys_handler, std::chrono::microseconds timeout);
};


/// @see seomio

