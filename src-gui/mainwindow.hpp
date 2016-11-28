#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTcpSocket>
#include <QVector>


#include "addressdialog.hpp"
#include "dataeater.hpp"
#include "commandexecutor.hpp"
#include "tunserver_process.hpp"


namespace Ui {
class MainWindow;
}

class commandExecutor;

class MainWindow final : public QMainWindow
{
	Q_OBJECT
public:
	static std::shared_ptr<MainWindow> create_shared_ptr();
	MainWindow (MainWindow &&other);
	~MainWindow();

	void start_tunserver(std::vector <peer_reference> &peer_list, const QString &tunserver_path);

	void SavePeers(QString file_name);
	void add_to_debug_window(const std::string &message);
	void show_peers(const std::vector<std::string> &peers);

public slots:

	void add_host_info(QString host, uint16_t port);
	void showDebugPage(QByteArray &pageCode);


private slots:
	void on_connectButton_clicked();
	void on_plusButton_clicked();
	void on_minusButton_clicked();
	void on_actionDebug_triggered();
	void on_ping_clicked();
	void on_run_tunserver_clicked();

	void update_peer_list(QString peer);

private:
	std::shared_ptr<commandExecutor> m_cmd_exec;

	Ui::MainWindow *ui;
	std::unique_ptr<tunserverProcess> m_tun_process;
	addressDialog *m_dlg;
	explicit MainWindow(QWidget *parent = nullptr);

signals:
	void ask_for_peerlist();
};


#endif // MAINWINDOW_H
