#include "c_server_msg_parser.hpp"

c_message c_server_msg_parser::parse (const char *data, size_t length) {
	c_message result;
	size_t i = parse_crypto_data(data, result, length);
	if (!result.is_ok)
		return result;

	result.org_msg_start = i;

	for (; i < length && data[i] != ' ' && data[i] != ':'; ++i)
		result.command += data[i];

	if (data[i] != ':') {
		++i;
		for (size_t line_id = 0; i < length && line_id < 10 && data[i] != ':'; ++i) {
			if (data[i] == ' ') {
				++line_id;
				continue;
			}
			result.Line[line_id] += data[i];
		}
	}

	for (++i; i < length; ++i)
		result.sender_nickname += data[i];

	return result;
}

size_t c_server_msg_parser::parse_crypto_data (const char *data, c_message &msg, size_t length) {
	if (length <= 3) {
		msg.is_ok = false;
		return 0;
	}

	if (data[0] != 'e' || data[1] != 'd')
		return 0;

	const size_t bufsize = 2;
	size_t i;

	for (i = 1; data[i * 2] != ':' && i * 2 < length; ++i) {
		char buffer[bufsize + 1];

		std::strncpy(buffer, data + i * 2, bufsize);

		buffer[bufsize] = '\0';

		int num = std::stoi(string(buffer), nullptr, 16);

		msg.Crypto_data[i - 1] = (unsigned char)num;
	}

	if (i * 2 >= length) {
		msg.is_ok = false;
		return 0;
	}

	return (i * 2) + 1;
}
