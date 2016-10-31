#include "peering_stats.hpp"
#include <sstream>
#include <iomanip>

using namespace std;

c_peering_stats::c_peering_stats():
    m_size_of_sent_data(0)
  ,m_size_of_read_data(0)
  ,m_number_of_sent_packets(0)
  ,m_number_of_read_packets(0)
  ,m_connection_time(chrono::system_clock::now())
  ,m_read_data_stats(60, 0)
  ,m_sent_data_stats(60, 0)
  ,m_time_read_data(chrono::system_clock::now())
  ,m_time_sent_data(chrono::system_clock::now())
 {}

void c_peering_stats::update_sent_stats(size_t size_of_sent_data){
    m_size_of_sent_data += size_of_sent_data;
    m_number_of_sent_packets++;
}

void c_peering_stats::update_read_stats(size_t size_of_read_data){
    m_size_of_read_data += size_of_read_data;
    m_number_of_read_packets++;
}

size_t c_peering_stats::get_size_of_sent_data() const{
    return m_size_of_sent_data;
}

size_t c_peering_stats::get_size_of_read_data() const{
    return m_size_of_read_data;
}

long c_peering_stats::get_number_of_sent_packets() const{
    return m_number_of_sent_packets;
}

long c_peering_stats::get_number_of_read_packets() const{
    return m_number_of_read_packets;
}

string c_peering_stats::get_connection_time() const{
    ostringstream out;
    auto now = chrono::system_clock::now();
    long seconds = chrono::duration<double, milli>(now - m_connection_time).count() / 1000;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    seconds %= 60;
    minutes %= 60;
    out << setfill('0') << setw(2) << hours << ':' << setfill('0') << setw(2) << minutes << ':' << setfill('0') << setw(2) << seconds;
    return out.str();
}

void c_peering_stats::reset_connection_time(){
    m_connection_time = chrono::system_clock::now();
}
