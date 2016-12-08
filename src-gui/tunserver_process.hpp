#ifndef TUNSERVERPROCESS_HPP
#define TUNSERVERPROCESS_HPP

#include <memory>
#include <QProcess>
#include <QStringList>

struct peer_reference {
	std::string m_ipv4;
	int m_port;
	std::string m_ipv6;

	std::string to_string() const;

	static peer_reference get_validated_ref(std::string ref);
};

class tunserverProcess final : public QObject {
	Q_OBJECT

public:
	tunserverProcess();
	~tunserverProcess();

	void run();
	void kill();

	std::vector<peer_reference> get_peer_list() const;

	void add_peer(const peer_reference &ref);
	void del_peer(const peer_reference &ref);
	void add_address(QString address);

public slots:
	void onProcessInfo();
	void onProcessError();

private:
	QString get_path();
	QStringList prepare_params();

	std::vector <peer_reference> m_peer_lst;
	std::unique_ptr<QProcess> m_tunserver_process;

	bool is_alive();
};

#endif // TUNSERVERPROCESS_HPP
