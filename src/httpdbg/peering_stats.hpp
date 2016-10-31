#pragma once
#ifndef PEERING_STATS
#define PEERING STATS 1

#include <ctime>

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

private:
    std::size_t m_size_of_sent_data;
    std::size_t m_size_of_read_data;
    long m_number_of_sent_packets;
    long m_number_of_read_packets;
};

#endif
