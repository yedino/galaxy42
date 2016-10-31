#pragma once
#ifndef PEERING_STATS
#define PEERING STATS 1

#include <ctime>
#include <chrono>
#include <string>

class c_peering_stats
{
public:
    c_peering_stats();
    void update_sent_stats(std::size_t size_of_sent_data);
    void update_read_stats(std::size_t size_of_read_data);
    std::size_t get_size_of_sent_data() const;
    std::size_t get_size_of_read_data() const;
    long get_number_of_sent_packets() const;
    long get_number_of_read_packets() const;
    std::string get_connection_time() const;
    void reset_connection_time();

private:
    std::size_t m_size_of_sent_data;
    std::size_t m_size_of_read_data;
    long m_number_of_sent_packets;
    long m_number_of_read_packets;
    std::chrono::time_point<std::chrono::system_clock> m_connection_time;
    boost::circular_buffer<int> m_read_data_stats;
    boost::circular_buffer<int> m_sent_data_stats;
    long m_time_read_data;
    long m_time_sent_data;
};

#endif
