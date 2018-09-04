#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTcpSocket>
#include <QVector>
#include <QtMultimedia/QSound>

#include <memory>
#include <utility>

#include "rpccounter.h"
#include "addressdialog.hpp"
#include "dataeater.hpp"
#include "commandexecutor.hpp"
#include "tunserver_process.hpp"
#include "ui_mainwindow.h"
#include "commandsender.h"
#include "tuntaprunner.h"


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
    void onPeerBanned(const QString &vip);
    void runTunTap();
	
	void setIps(const QString &ip,const QString &vip);

public slots:

	void add_host_info(QString host, uint16_t port);
    void showDebugPage(QByteArray &pageCode);
    void onCreateGalaxyConnection();

    void onGetMyInvitatiom(std::string );
    void errorNotification(QString err);

    void addDebugInfo(const QString &str);

    void onBanPeer(const QString &vip);		//! slot of ban one peer
    void onRemovePeer(const QString &vip);		//! slot of remove one peer
    void onAddPeer(const QString &vip);		//! slot of add peer
    void onSendMessage(const QString &vip,const QString &msg);	//! slot that sended message (no rpc command now - so not working)
    void onFindPeer(const QString &vip);			//! slot that loogking for peer (no rpc command now - not working)

    void onPeerRemoved(const QString &vip);
    void onPeerAdded(const QString &vip);
    void onDeletePeer(QString &vip);

    void onBanAll();
    void onDeleteAll();

    bool isWorking();
    void setBtc(uint64_t btc);

    void setDebugInfo(const QString &str);

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
    void onNetConnected(bool);

private:
    commandExecutor *m_cmd_exec;

    Ui::MainWindow* ui;
	std::unique_ptr<tunserverProcess> m_tun_process;
    std::unique_ptr<addressDialog> m_dlg;

    void deletePeer(const std::string &peer_id);

    CommandSender* m_sender;
    StatusForm *m_status_form;
    TunTapRunner *m_tuntap_runner;

    uint64_t m_last_btc_value;

    void runTunTap(const QString& run_,const QString& script );

signals:
	void ask_for_peerlist();
};


#endif // MAINWINDOW_H
