#include "dataeater.hpp"
#include <iostream>
#include <cstring>

#include "trivialserialize.hpp"

void dataeater::eat(const std::vector<uint8_t> &data) {
	for(const auto &i:data) m_internal_buffer.push(i);
}

void dataeater::eat(const std::string &data) {
	static_assert(sizeof(char) == sizeof(uint8_t), "size of char are different than size of uint8_t");
	for(const auto i:data) {
		eat(i);
	}
}

void dataeater::eat(const char &ch) {
	static_assert(sizeof(char) == sizeof(uint8_t), "size of char are different than size of uint8_t");
	uint8_t que_ele;
	memcpy(&que_ele,&ch,sizeof(uint8_t));
	m_internal_buffer.push(que_ele);
}

void dataeater::process() {

	if(!m_is_processing) {
		processFresh();
	} else {
		continiueProcessing();
	}
}

uint16_t dataeater::pop_msg_size() {
	uint16_t msg_size;
	msg_size = static_cast<uint16_t>(m_internal_buffer.front() << 8);
	m_internal_buffer.pop();
	msg_size += m_internal_buffer.front();
	m_internal_buffer.pop();
	return msg_size;
}

bool dataeater::processFresh() {
	// change 4 to 2 because of no 0xff at the begin of packet
	if (m_internal_buffer.size() < 2){
		return false;
	}

	// Is it really neccessary?
	//if(char (m_internal_buffer.front()) != char(0xff)) {	//frame should start with 0xff. If not - something goes wrong
	//	m_internal_buffer.pop();
	//	return false;
	//}

	m_frame_size = pop_msg_size();

	std::cout << "qframe size = " << m_frame_size << std::endl;

	m_current_index = 0;
	continiueProcessing();
	return m_is_processing = true;
}

bool dataeater::continiueProcessing() {

	while (true) {
		if (m_frame_size == m_current_index) {
			m_commands_list.push(m_last_command);
			m_last_command.clear();
			m_is_processing= false;
			processFresh();
			return true;
		} else {
			if(m_internal_buffer.empty()) break;
			m_last_command.push_back(m_internal_buffer.front()); m_internal_buffer.pop();
		}
		m_current_index++;
	}
	return m_is_processing;
}

std::string dataeater::getLastCommand() {
	if(m_commands_list.empty()) {
		return std::string();
	}
	return std::string (m_commands_list.back());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
