#include "dataeater.hpp"
#include <iostream>
#include <cstring>
#include <QDebug>

#include "trivialserialize.hpp"

dataeater::dataeater(): m_is_processing(false){}

uint16_t dataeater::pop_msg_size() {
	uint16_t msg_size;
	msg_size = static_cast<uint16_t>(m_internal_buffer.front() << 8);
	m_internal_buffer.pop();
	msg_size += m_internal_buffer.front();
	m_internal_buffer.pop();
	return msg_size;
}

void dataeater::processFresh() {}

void dataeater::continiueProcessing() {
	if(!m_is_processing) {
		// change 4 to 2 because of no 0xff at the begin of packet
		if (m_internal_buffer.size() < 2) {
			return;
		}

		// Is it really neccessary?
		//if(char (m_internal_buffer.front()) != char(0xff)) {	//frame should start with 0xff. If not - something goes wrong
		//	m_internal_buffer.pop();
		//	return false;
		//}

		m_frame_size = pop_msg_size();

		qDebug() << "qframe size = " << m_frame_size;

		m_current_index = 0;
		//continiueProcessing();
		m_is_processing = true;

	}

	while (true) {
		if (m_frame_size == m_current_index) {
			m_commands_list.push(m_last_command);
			m_last_command.clear();
			m_is_processing = false;
			processFresh();
			return;
		} else {
			if(m_internal_buffer.empty()) break;
			m_last_command.push_back(static_cast<char>(m_internal_buffer.front())); m_internal_buffer.pop();
		}
		m_current_index++;
	}

}

std::string dataeater::getLastCommand() {
	if(m_commands_list.empty()) {
		return std::string();
	}
	return std::string (m_commands_list.back());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
