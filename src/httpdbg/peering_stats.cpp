#include "peering_stats.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

c_data_tramsmission_buffer::c_data_tramsmission_buffer(int buffer_size, int interval_in_milisec):
    m_data_sent_buffer(buffer_size, 0)
  ,m_data_read_buffer(buffer_size, 0)
  ,m_packets_sent_buffer(buffer_size, 0)
  ,m_packets_read_buffer(buffer_size, 0)
  ,m_last_sent(0)
  ,m_last_read(0)
  ,m_interval(interval_in_milisec)
  ,m_reference_time(chrono::system_clock::now())
{}

void c_data_tramsmission_buffer::update_sent_buffer(size_t data_size){
    auto now = chrono::system_clock::now();
    long units = chrono::duration<double, milli>(now - m_reference_time).count() / m_interval;
    long distance = units - m_last_sent;
    m_last_sent = units;
    for(int i=0; i<distance; i++)
    {
        m_data_sent_buffer.push_back(0);
        m_packets_sent_buffer.push_back(0);
    }
    if(data_size)
    {
        m_data_sent_buffer.back() += data_size;
        m_packets_sent_buffer.back()++;
    }
}

void c_data_tramsmission_buffer::update_read_buffer(size_t data_size){
    auto now = chrono::system_clock::now();
    long units = chrono::duration<double, milli>(now - m_reference_time).count() / m_interval;
    long distance = units - m_last_read;
    m_last_read = units;
    for(int i=0; i<distance; i++)
    {
        m_data_read_buffer.push_back(0);
        m_packets_read_buffer.push_back(0);
    }
    if(data_size)
    {
        m_data_read_buffer.back() += data_size;
        m_packets_read_buffer.back()++;
    }
}

string c_data_tramsmission_buffer::get_data_buffer_as_js_str(string var){
    ostringstream out;
    update_read_buffer(0);
    update_sent_buffer(0);
    out << " var d" << var << " = google.visualization.arrayToDataTable([['Seconds', 'Read data', 'Sent data'],";
    for(unsigned i=0; i<m_data_read_buffer.size(); i++)
        out << "['" << i << "', " << m_data_read_buffer[i] << ", " << m_data_sent_buffer[i] << "],";
    string str = out.str();
    str[str.size()-1] = ' ';
    return str + "]);";
}

string c_data_tramsmission_buffer::get_packets_buffer_as_js_str(string var){
    ostringstream out;
    update_read_buffer(0);
    update_sent_buffer(0);
    out << " var p" << var << " = google.visualization.arrayToDataTable([['Seconds', 'Read packets', 'Sent packets'],";
    for(unsigned i=0; i<m_data_read_buffer.size(); i++)
        out << "['" << i << "', " << m_packets_read_buffer[i] << ", " << m_packets_sent_buffer[i] << "],";
    string str = out.str();
    str[str.size()-1] = ' ';
    return str + "]);";
}

string c_data_tramsmission_buffer::get_charts_as_js_str(string var, bool display_big_chart){
    ostringstream out;
    out << "var cd" << var << " = new google.visualization.AreaChart(document.getElementById('cd_div" << var <<"'));";
    out << "cd" << var << ".draw(d" << var << ", data_options);";
    out << "var cp" << var << " = new google.visualization.AreaChart(document.getElementById('cp_div" << var <<"'));";
    out << "cp" << var << ".draw(p" << var << ", packets_options);";
    if(display_big_chart)
    {
        out << "var bcd" << var << " = new google.visualization.AreaChart(document.getElementById('bcd_div" << var <<"'));";
        out << "bcd" << var << ".draw(d" << var << ", data_options_big);";
        out << "var bcp" << var << " = new google.visualization.AreaChart(document.getElementById('bcp_div" << var <<"'));";
        out << "bcp" << var << ".draw(p" << var << ", packets_options_big);";
    }
    return out.str();
}

c_peering_stats::c_peering_stats():
    m_size_of_sent_data(0)
  ,m_size_of_read_data(0)
  ,m_number_of_sent_packets(0)
  ,m_number_of_read_packets(0)
  ,m_connection_time(chrono::system_clock::now())
  ,m_data_buffer(120, 1000)
 {}

void c_peering_stats::update_sent_stats(size_t size_of_sent_data){
    m_data_buffer.update_sent_buffer(size_of_sent_data);
    m_size_of_sent_data += size_of_sent_data;
    m_number_of_sent_packets++;
}

void c_peering_stats::update_read_stats(size_t size_of_read_data){
    m_data_buffer.update_read_buffer(size_of_read_data);
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

c_data_tramsmission_buffer & c_peering_stats::get_data_buffer(){
    return m_data_buffer;
}
