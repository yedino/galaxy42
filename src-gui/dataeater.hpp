#ifndef DATAEATER_H
#define DATAEATER_H


#include <memory>
#include <queue>
#include <QByteArray>

class dataeater {
public:
	dataeater(): m_is_processing(false){;}

	void eat(const std::vector<uint8_t> &data);
	void eat(const std::string &data);
	void eat(const char &ch);
	void process();
	bool hasNextCommand();
	std::string getLastCommand();

private:
	std::queue<uint8_t> m_internal_buffer;
	std::queue<std::string> m_commands_list;
	std::string m_last_command;

	uint16_t pop_msg_size();
	bool processFresh();
	bool continiueProcessing();

	bool m_is_processing;

	int m_frame_size;
	int m_current_index;
};


#endif // DATAEATER_H
