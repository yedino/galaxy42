#ifndef DATAEATER_H
#define DATAEATER_H


#include <memory>
#include <queue>
#include <QByteArray>

class dataeater final {
public:
	dataeater();
	/**
	 * @brief eat copy data into internal buffer
	 * @param data
	 */
	template <class T>
	void eat(const T &data);

	std::string getLastCommand(); ///< returns empty string or received data + crypto_auth_hmacsha512_BYTES bytes of hmac

private:
	std::queue<uint8_t> m_internal_buffer;
	std::queue<std::string> m_commands_list;
	std::string m_last_command;

	uint16_t pop_msg_size();
	void continiueProcessing();

	bool m_is_processing;

	unsigned int m_frame_size;
	size_t m_current_index;
};

template <class T>
void dataeater::eat(const T &data) {
	static_assert(sizeof(char) == sizeof(int8_t), "size of char are different than size of int8_t");
	static_assert(sizeof(unsigned char) == sizeof(uint8_t), "size of unsigned char are different than size of uint8_t");
	static_assert(sizeof(typename T::value_type) == sizeof(char), "bad container type size (size should be equal 1)");
	for (const auto &c : data) {
			m_internal_buffer.push(static_cast<uint8_t>(c));
	}
	continiueProcessing();
}


#endif // DATAEATER_H
