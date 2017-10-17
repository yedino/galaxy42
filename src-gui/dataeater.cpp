#include "dataeater.hpp"
#include <iostream>
#include <cstring>
#include <QDebug>
#include <sodium.h>


dataeater::dataeater(): m_is_processing( false ) {}

uint16_t dataeater::pop_msg_size() {
    uint16_t msg_size;
    msg_size = static_cast<uint16_t>( m_internal_buffer.front() << 8 );
    m_internal_buffer.pop();
    msg_size += m_internal_buffer.front();
    m_internal_buffer.pop();
    return msg_size;
}

void dataeater::continiueProcessing() {
    // reading 2 bytes of size
    if( !m_is_processing ) {
        // change 4 to 2 because of no 0xff at the begin of packet
        if ( m_internal_buffer.size() < 2 ) {
            return;
        }

        m_frame_size = pop_msg_size();
        qDebug() << "qframe size = " << m_frame_size;
        m_current_index = 0;
        m_is_processing = true;
    }

    // reading message data + authenticator
    while ( true ) {
        if ( m_current_index == ( m_frame_size + crypto_auth_hmacsha512_BYTES ) ) {
            m_commands_list.push( m_last_command );
            m_last_command.clear();
            m_is_processing = false;
            return;
        } else {
            if( m_internal_buffer.empty() ) break;
            m_last_command.push_back( static_cast<char>( m_internal_buffer.front() ) );
            m_internal_buffer.pop();
        }
        m_current_index++;
    }
}

std::string dataeater::getLastCommand() {
    if( m_commands_list.empty() ) {
        return std::string();
    }
    return std::string( m_commands_list.back() );
}

