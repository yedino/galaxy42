#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTcpSocket>
#include <QVector>
#include <memory>
#include <utility>

#include "rpccounter.h"
#include "addressdialog.hpp"
#include "dataeater.hpp"
#include "commandexecutor.hpp"
#include "tunserver_process.hpp"
#include "ui_mainwindow.h"
#include "commandsender.h"

class commandExecutor;
class CommandSender;
class StatusForm;

namespace Ui {
class MainWindow;
}



class MainWindow final : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	static std::shared_ptr<MainWindow> create_shared_ptr();

	void start_tunserver(std::vector <peer_reference> &peer_list, const QString &tunserver_path);

	void SavePeers(QString file_name);
	void add_to_debug_window(const std::string &message);
    void show_peers(const std::vector<std::string> &peers);
    void sendLocalIps(QStringList ips);

    void startNewCrpcConnection(const QString &host, uint port);

    StatusForm *GetStatusObject();
public slots:

	void add_host_info(QString host, uint16_t port);
    void showDebugPage(QByteArray &pageCode);
    void onCreateGalaxyConnection();

    QStringList getLocalIps();
    QStringList getLocalVips();

    void onGetMyInvitatiom(std::string );
    void errorNotification(QString err);

    void addDebugInfo(const QString &str);

    void onBanBeer(const QString &vip);		//! funkcja wywolywana na Ban  z listy peer
    void onRemovePeer(const QString &vip);		//! funkcja wywolywana na Remove z listy peer
    void onAddPeer(const QString &vip);		//! funkcja wywolywana na add z listy peer
    void onSendMessage(const QString &vip,const QString &msg);	//! funkcja wywolywana na send msg z listy peer
    void onFindPeer(const QString &vip);			//! funkcja wywolywana na find z listy peer

    void onPeerRemoved(const QString &vip);
    void onPeerAdded(const QString &vip);

    void onBanAll();
    void onDeleteAll();

private slots:

    void onAddPerrToList(const QString &peer);
    void initSettings();
    void loadSettings();

	void on_connectButton_clicked();
	void on_plusButton_clicked();
	void on_minusButton_clicked();
	void on_actionDebug_triggered();
	void on_ping_clicked();
	void on_run_tunserver_clicked();
	void update_peer_list(QString peer);
    void on_banButton_clicked();
    void connectToNet(QString);
    void createNet();

    void onAllowFriend(bool);
    void onAllowPeer(bool);
    void on_actionsettings_triggered();

private:
    commandExecutor *m_cmd_exec;

    Ui::MainWindow* ui;
	std::unique_ptr<tunserverProcess> m_tun_process;
    std::unique_ptr<addressDialog> m_dlg;

    void deletePeer(const std::string &peer_id);

    QString m_host_port;
    QString m_host_ip;
    CommandSender* m_sender;
    StatusForm *m_status_form;

signals:
	void ask_for_peerlist();
};


#endif // MAINWINDOW_H
