#include "peering_stats.hpp"

c_peering_stats::c_peering_stats():
    m_size_of_sent_data(0)
  ,m_size_of_read_data(0)
  ,m_number_of_sent_packets(0)
  ,m_number_of_read_packets(0)
{}

void c_peering_stats::update_sent_stats(std::size_t size_of_sent_data){
    m_size_of_sent_data += size_of_sent_data;
    m_number_of_sent_packets++;
}

void c_peering_stats::update_read_stats(std::size_t size_of_read_data){
    m_size_of_read_data += size_of_read_data;
    m_number_of_read_packets++;
}

std::size_t c_peering_stats::get_size_of_sent_data() const{
    return m_size_of_sent_data;
}

std::size_t c_peering_stats::get_size_of_read_data() const{
    return m_size_of_read_data;
}

long c_peering_stats::get_number_of_sent_packets() const{
    return m_number_of_sent_packets;
}

long c_peering_stats::get_number_of_read_packets() const{
    return m_number_of_read_packets;
}
