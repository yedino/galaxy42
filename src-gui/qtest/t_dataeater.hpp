#include <QtTest/QtTest>
#include "../dataeater.hpp"
#include "../netclient.hpp"
#include "../json.hpp"

using nlohmann::json;

class Test_dataeater: public QObject {
    Q_OBJECT

private slots:
	void eat_simple_packet() {
		json j_msg  {
							{"cmd", "ping"},
							{"msg", "pong"}
						};
		std::string j_parsed = j_msg.dump();

		dataeater dataeater_test;
		dataeater_test.eat(netClient::serialize_msg(j_parsed).toStdString());
		dataeater_test.process();

		std::string processed = dataeater_test.getLastCommand();

		QVERIFY(processed == j_parsed);
	}
	void eat_many_packets() {
		json j_msg  {
							{"cmd", "ping"},
							{"m", "p"}
						};
		std::string j_parsed = j_msg.dump();

		dataeater dataeater_test;

		for(int i = 0; i < 3; ++i) {
			dataeater_test.eat(netClient::serialize_msg(j_parsed).toStdString());
			dataeater_test.process();
		}

		for(int i = 0; i < 3; ++i) {
			std::string processed = dataeater_test.getLastCommand();
			QVERIFY(processed == j_parsed);
		}
	}
	void eat_cutted_packet() {
		json j_msg  {
							{"cmd", "ping"},
							{"msg", "pong"}
						};
		std::string j_parsed = j_msg.dump();

		dataeater dataeater_test;
		std::string packet = netClient::serialize_msg(j_parsed).toStdString();

		std::string part1 = packet.substr(0,10);
		std::string part2 = packet.substr(10);

		dataeater_test.eat(part1);
		dataeater_test.process();
		dataeater_test.eat(part2);
		dataeater_test.process();

		std::string processed = dataeater_test.getLastCommand();

		QVERIFY(processed == j_parsed);
	}
	void eat_packet_char_by_char() {
		json j_msg  {
							{"cmd", "ping"},
							{"msg", "pong"}
						};
		std::string j_parsed = j_msg.dump();

		dataeater dataeater_test;
		std::string packet = netClient::serialize_msg(j_parsed).toStdString();

		for(const auto &i : packet) {
			// eat char by char
			dataeater_test.eat(i);
			dataeater_test.process();
		}

		std::string processed = dataeater_test.getLastCommand();

		QVERIFY(processed == j_parsed);
	}
};
