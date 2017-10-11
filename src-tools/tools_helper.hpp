#include <vector>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <chrono>
#include <thread>
#include <atomic>

template<typename TFloat>
TFloat mediana(std::vector<TFloat> tab) {
	if (tab.size()<=0) return 0;
	assert(tab.size()>=1);

	// std::nth_element(tab.begin(), tab.begin() + tab.size()/2, tab.end()); // odd

	sort(tab.begin(), tab.end());

	if (1==(tab.size() % 2 )) {
			return tab.at( tab.size()/2 ); // mediana odd    [10 *20 30]
	} else {
		auto a = tab.at( tab.size()/2 -1 );
		auto b = tab.at( tab.size()/2  );

		return (a+b)/2.;
		// mediana odd    [10 *20 *30 40]
	}
}

template<typename TFloat>
TFloat corrected_avg(std::vector<TFloat> tab) {
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

	return std::accumulate( & tab.at(pos1) , & tab.at(pos2) , 0 ) / len;
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
};

c_timerfoo::c_timerfoo(int step_till_reset) : m_time_started(t_mytime{}), m_count(0), m_size(0),
m_speed_now(0), m_speed_pck_now(0), m_step_nr(0),
m_step_till_reset(step_till_reset),
m_best_result(0)
{
}

void c_timerfoo::reset() {
	m_time_started=t_mytime{};
	m_count=0;
	m_size=0;
	m_speed_now=0;
	m_speed_pck_now=0;
	{
		std::lock_guard< std::mutex > lg(m_mutex);
		m_speed_tab.clear();
	}
	m_step_nr=0;
}

void c_timerfoo::step() {
	this->calc_speed_now();
	bool should_reset=false;
	{
		std::lock_guard< std::mutex > lg(m_mutex);
		m_speed_tab.push_back( get_speed() ); // *** add
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

	if (this->m_count == 0) {
		t_mytime time_now( std::chrono::steady_clock::now() );
		t_mytime time_zero;
		this->m_time_started.compare_exchange_strong(
			time_zero,
			time_now
		);
	}

	this->m_count += count;
	this->m_size += size_totall;
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
	double current_size_speed  = current_size  / ellapsed_sec; // in B/s
	double current_count_speed = current_count / ellapsed_sec; // in B/s

	const double mega = (1*1000*1000);

	m_speed_now = current_size_speed * 8. / mega;
	m_speed_pck_now = current_count_speed / mega;
}

void c_timerfoo::calc_avg() {
	std::lock_guard< std::mutex > lg(m_mutex);
	m_speed_avg1 = mediana( m_speed_tab );
	m_speed_avg2 = corrected_avg( m_speed_tab );
}

void c_timerfoo::print_info(std::ostream & ostr) const {
	std::lock_guard< std::mutex > lg(m_mutex);

	t_my_size current_size = this->m_size.load();
	t_my_count current_count = this->m_count.load();

	int detail=0;
	if (detail>=2) { ostr << std::setw(9) << current_size  << " B "; }
	ostr << std::setw(4) << m_speed_avg1
		<< " (" << std::setw(4) << m_speed_now
		<< " best=" << std::setw(4) << m_best_result << ")"
		<< " Mb/s" ;
	ostr << " ";
	if (detail>=1) { ostr << std::setw(6) << current_count << " p "; }
	ostr << std::setw(4) << m_speed_pck_now << " Mp/s" ;
}

std::ostream & operator<<(std::ostream & ostr, c_timerfoo & timer) {
	timer.print_info(ostr);
	return ostr;
}


