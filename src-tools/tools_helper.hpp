#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#define DEBUG_TOOLS_HELPER 0

template<typename TData>
long double mediana(std::vector<TData> tab) {
	static_assert(std::is_arithmetic<TData>::value, "type must be floating point");
	if (tab.size()<=0) return 0;
	assert(tab.size()>=1);

	// std::nth_element(tab.begin(), tab.begin() + tab.size()/2, tab.end()); // odd

	sort(tab.begin(), tab.end());

	if (1==(tab.size() % 2 )) {
			return tab.at( tab.size()/2 ); // mediana odd    [10 *20 30]
	} else {
		auto a = tab.at( tab.size()/2 -1 );
		auto b = tab.at( tab.size()/2  );

		return (a+b) / static_cast<long double>(2.0);
		// mediana odd    [10 *20 *30 40]
	}
}

template<typename TData>
long double corrected_avg(std::vector<TData> tab) {
	auto size = tab.size();
	if (size<=0) return 0;
	if (size<=1) return tab.at(0);

	auto margin = size/4;
	if (margin<1) margin=1;

	auto pos1 = 0+margin, pos2=size-1-margin;
	if (pos2 < pos1) return 0; // too small; can't substract
	int len = pos2-pos1;
	if (!(len>=2)) return 0; // too small yet

	//	std::cerr<<"Avg from "<<pos1<<" to " <<pos2 << " len="<<len<<" size="<<size<<std::endl;

	return std::accumulate( & tab.at(pos1) , & tab.at(pos2) , static_cast<long double>(0.0) ) / len;
}

template<typename TData>
long double average(std::vector<TData> tab) {
	auto acc = std::accumulate( tab.begin() , tab.end() , static_cast<long double>(0.0) );
	return acc / static_cast<long double>(tab.size());
}

/// simple time value
struct t_mytime {
	using t_timevalue = std::chrono::time_point<std::chrono::steady_clock>;
	t_timevalue m_time;
	t_mytime() noexcept { }
	t_mytime(std::chrono::time_point<std::chrono::steady_clock> time_) noexcept { m_time = time_; }
};

/**
Timer that is incremented by .add(), then periodically .step() is called and optionally calc_speed_now and cout<<this;
It should be thread-safe to use (maybe not well tested yet)
*/
class c_timerfoo {
	public:
		using t_my_count = uint64_t;
		using t_my_size = uint64_t;

		c_timerfoo(int step_till_reset);
		void add(t_my_count count, t_my_size size_totall) noexcept; ///< e.g. (3,1024) means we got 3 packets, that in sum have size 1024 B
		std::string get_info() const ;
		void print_info(std::ostream & ostr) const ;

		void step(); ///< one step of the cycle
		void calc_speed_now();
		double get_speed() const;

		void set_debug(int value);
	private:
		void reset();
		void calc_avg();

		std::atomic<t_mytime> m_time_started;
		std::atomic<t_my_count> m_count;
		std::atomic<t_my_size> m_size;

		std::atomic<double> m_speed_now, m_speed_pck_now; ///< calculated speed. Mbit/s, pck/s
		mutable std::mutex m_mutex; ///< protects some stats like speed_tab

		std::vector<double> m_speed_tab; ///< under mutex!
		int m_step_nr; ///< under mutex!
		int m_step_till_reset; ///< under mutex!
		double m_speed_avg1, m_speed_avg2; ///< under mutex

		double m_best_result; ///< best so far result, taken from average of viable size. is not reseted.

		int m_debug;

		double m_ellapsed_used_in_current; ///< what was the ellapsed time used in calculating most recent current speed
		/// (e.g. if low then speed is bad)

		std::atomic<t_my_size> m_total_bytes;
		std::atomic<t_mytime> m_total_start;
		double m_total_speed_bytes;
};

c_timerfoo::c_timerfoo(int step_till_reset) : m_time_started(t_mytime{}), m_count(0), m_size(0),
m_speed_now(0), m_speed_pck_now(0), m_step_nr(0),
m_step_till_reset(step_till_reset),
m_best_result(0),
m_debug(false),
m_ellapsed_used_in_current(0),
m_total_bytes(0),
m_total_start(t_mytime{}),
m_total_speed_bytes(0)
{
	reset(); // to make sure all is nice

	set_debug(DEBUG_TOOLS_HELPER);
}

void c_timerfoo::set_debug(int value) { m_debug = value; }

void c_timerfoo::reset() {
	if (m_debug) {
		std::cerr << "Reseting timer " << (void*)(this) << " ------------------- \n";
	}

	m_time_started = std::chrono::steady_clock::now();
	m_count=0;
	m_size=0;
	m_speed_now=0;
	m_speed_pck_now=0;
	{
		std::lock_guard< std::mutex > lg(m_mutex);
		m_speed_tab.clear();
	}
	m_step_nr=0;
	m_ellapsed_used_in_current=0;
}

void c_timerfoo::step() {
	this->calc_speed_now();
	bool should_reset=false;
	{
		std::lock_guard< std::mutex > lg(m_mutex);
		auto speed_now = get_speed(); // *** current speed
		if ( m_ellapsed_used_in_current > 0.5 ) { // discard samples from too small time periods to avoid noise
			m_speed_tab.push_back( speed_now ); // *** add
		}
		if (m_debug) {
			std::cerr << "Adding sample to timer " << (void*)(this) << " "
				<< "speed_now=" << speed_now << " "
				<< "array: {";
			for (auto const & x : m_speed_tab) std::cerr << x << " ";
			std::cerr << "}";
			std::cerr	<< "\n";
		}
		++m_step_nr;
		if ( (m_step_till_reset>0) && (m_step_nr >= m_step_till_reset) ) {
			should_reset=true;

			if ( ( m_size > 5 * 1000 * 1000 ) && (m_count > 10*1000 ) ) {
				m_best_result = std::max( m_speed_avg1 , m_best_result );
			}
		}
	}
	this->calc_avg();
	if (should_reset) this->reset();
}

double c_timerfoo::get_speed() const { return m_speed_now; }

void c_timerfoo::add(t_my_count count, t_my_size size_totall) noexcept {
	// [counter]
	if (m_debug >= 2) {
		std::cerr << "*** ADD *** Timer - adding data - (" << ((void*)this) << "), adding size="<<size_totall<<" count="<<count
			<< " after add: m_size=" << m_size << ", m_count=" << m_count << "\n";
	}

	this->m_count += count;
	this->m_size += size_totall;

	if( m_total_bytes == 0) {
		m_total_start = std::chrono::steady_clock::now();
	}
	this->m_total_bytes += size_totall;
}

std::string c_timerfoo::get_info() const {
	std::ostringstream oss;
	print_info(oss);
	return oss.str();
}

void c_timerfoo::calc_speed_now() {
	auto time_now = std::chrono::steady_clock::now();
	auto time_started = this->m_time_started.load().m_time;
	t_my_size current_size = this->m_size.load();
	t_my_count current_count = this->m_count.load();

	double ellapsed_sec = (
		std::chrono::duration_cast<std::chrono::microseconds>(time_now - time_started) ).count()
		/ (1000.*1000. );

	m_ellapsed_used_in_current = ellapsed_sec;

	double current_size_speed  = current_size  / ellapsed_sec; // in B/s
	double current_count_speed = current_count / ellapsed_sec; // in B/s

	const double mega = (1*1000*1000);

	if (m_debug) {
		std::cerr << std::setprecision(4)  << std::fixed
			<< "Speed now in timer " << (void*)(this) << ": "
			<< " ellapsed=" << ellapsed_sec << " current_size=" << current_size
			<< " so speed=" << ( current_size_speed*8.f / (1000. * 1000.) ) << " Mbit/s"
		<< "\n";
	}

	m_speed_now = current_size_speed * 8. / mega;
	m_speed_pck_now = current_count_speed / mega;
}

void c_timerfoo::calc_avg() {
	std::lock_guard< std::mutex > lg(m_mutex);
	m_speed_avg1 = mediana( m_speed_tab );
	m_speed_avg2 = corrected_avg( m_speed_tab );

	auto time_now = std::chrono::steady_clock::now();
	auto ellapsed = time_now - m_total_start.load().m_time;
	double ellapsed_sec =
		std::chrono::duration_cast<std::chrono::microseconds>(ellapsed).count()
		/ (1000.*1000. );

	m_total_speed_bytes = (m_total_bytes*8 / ellapsed_sec) / (1000.*1000.);
}

void c_timerfoo::print_info(std::ostream & ostr) const {
	std::lock_guard< std::mutex > lg(m_mutex);

	t_my_size current_size = this->m_size.load();
	t_my_count current_count = this->m_count.load();

	int detail=0;
	if (detail>=2) { ostr << std::setw(9) << current_size  << " B "; }
	ostr << std::setw(4)
	  << "avg=" << m_speed_avg1
		<< " (now=" << std::setw(4) << m_speed_now << ")"
		<< " bestAvg=" << std::setw(4) << m_best_result << ""
		<< " Total=" << std::setw(4) << m_total_speed_bytes

		<< " Mb/s" ;
	ostr << " ";

	std::ofstream result_file("/tmp/result.txt");
	if (result_file.is_open())
	{
		result_file << m_total_speed_bytes;
		result_file.close();
	}
	if (detail>=1) { ostr << std::setw(6) << current_count << " p "; }
	ostr << std::setw(4) << m_speed_pck_now << " Mp/s" ;
}

std::ostream & operator<<(std::ostream & ostr, c_timerfoo & timer) {
	timer.print_info(ostr);
	return ostr;
}

// ============================================================================

/**
Mini timer that just collects data, and later appends them to main c_timerfoo
NOT THREAD SAFE - use in 1 thread! (usually as thread_local)
*/
class c_timeradd {
	public:
		using t_my_size = c_timerfoo::t_my_size;
		using t_my_count = c_timerfoo::t_my_count;

		/// @warning the parent_timer must live as long as this object, or ub!
		c_timeradd(c_timerfoo & parent_timer, t_my_count update_interval_count);

		void add(t_my_count count, t_my_size size_totall) noexcept; ///< e.g. (3,1024) means we got 3 packets, that in sum have size 1024 B

	private:
		t_my_count m_count;
		t_my_count m_update_interval_count;
		t_my_size m_size;
		c_timerfoo & m_parent_timer;
};

c_timeradd::c_timeradd(c_timerfoo & parent_timer, t_my_count update_interval_count)
:
m_count(0),
m_update_interval_count(update_interval_count),
m_size(0),
m_parent_timer(parent_timer)
{ }

void c_timeradd::add(t_my_count count, t_my_size size_totall) noexcept {
	m_count += count;
	m_size += size_totall;

	if ( m_count > m_update_interval_count ) {
		#if DEBUG_TOOLS_HELPER >= 2
		std::cerr << " timeradd (" << ((void*)this) << ") will now add to his main timer (because " << m_count << " > " << m_update_interval_count << " \n";
		#endif

		m_parent_timer.add( m_count , m_size ); // *** increment parent
		m_count = 0;
		m_size = 0;

		#if DEBUG_TOOLS_HELPER >= 2
		std::cerr << " timeradd (" << ((void*)this) << ") added to his main timer... Now count=" << m_count << " vs interval=" << m_update_interval_count << " \n\n";
		#endif
	}
}
