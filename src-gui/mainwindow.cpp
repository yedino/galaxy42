#include <QDebug>
#include <QProcess>
#include <QTcpSocket>
#include <QHostAddress>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "addressdialog.hpp"
#include "paramscontainer.hpp"
#include "dataeater.hpp"
#include "debugdialog.hpp"
#include "get_host_info.hpp"


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_tun_process(std::make_unique<tunserverProcess>()),
	m_dlg(nullptr)
{
	ui->setupUi(this);

	/* load peers from file - TODO
	ParamsContainer params;
	params.readParams("peers.json");
	m_peer_lst = params.getPeerList();

	for(auto it : m_peer_lst){
		QString peer_val = QString::fromStdString(it.m_ipv6);
		ui->peerListWidget->addItem(peer_val);
	}
	*/
}

std::shared_ptr<MainWindow> MainWindow::create_shared_ptr() {
	// TODO
	std::shared_ptr<MainWindow> ret(new MainWindow);// = std::make_shared<MainWindow>();
	ret->m_cmd_exec = commandExecutor::construct(ret);
	return ret;
}

MainWindow::MainWindow(MainWindow &&other) {
	if (this == &other)
		return;
	m_cmd_exec = std::move(other.m_cmd_exec);
	ui = other.ui;
	other.ui = nullptr;
	//m_tun_process = other.m_tunserver_process;
	//other.m_tunserver_process = nullptr;
	m_dlg = other.m_dlg;
	other.m_dlg = nullptr;
}

MainWindow::~MainWindow()
{
	delete ui;
	delete m_dlg;
}

void MainWindow::showDebugPage(QByteArray &pageCode) {

}

void MainWindow::on_plusButton_clicked() {

	m_dlg = new addressDialog(this);
	connect (m_dlg,SIGNAL(add_address(QString)),this,SLOT(update_peer_list(QString)));
	m_dlg->show();

}

void MainWindow::update_peer_list(QString peer) {

	m_tun_process->add_address(peer);

	ui->peerListWidget->clear();
	std::cout << "Peers:" << '\n';
	for (const auto &peer : m_tun_process->get_peer_list()) {
		std::cout << peer.to_string() << '\n';
		ui->peerListWidget->addItem(QString::fromStdString(peer.to_string()));
	}
}

void MainWindow::on_minusButton_clicked() {

	const auto &delete_list = ui->peerListWidget->selectedItems();
	if(delete_list.isEmpty()) return;
	try{
		// double validating
		m_tun_process->del_peer(peer_reference::get_validated_ref(delete_list.at(0)->text().toStdString()));

		ui->peerListWidget->removeItemWidget(delete_list.at(0));
        delete_list.at(0)->setText("");
        ui->peerListWidget->sortItems(Qt::DescendingOrder);
    } catch(...){
        qDebug()<<"co mam niby usunac?!";
    }
}

void MainWindow::on_run_tunserver_clicked() {
	m_tun_process->run();
}

void MainWindow::add_host_info(QString host, uint16_t port) {

	qDebug() << "Host: " << host << "Port:" << port << '\n';
	if (host.isEmpty()) {
		host = "127.0.0.1";
		qDebug() << "use default 'localhost (127.0.0.1)' host";
	}
	if (port == 0) {
		port = 42000;
		qDebug() << "use default '42000' port";
	}
	m_cmd_exec->startConnect(QHostAddress(host), port);
}

void MainWindow::on_connectButton_clicked() {
	hostDialog host_dialog;

	connect (&host_dialog, SIGNAL(host_info(QString, uint16_t)),
			 this, SLOT(add_host_info(QString, uint16_t)));

	if (host_dialog.exec() == QDialog::Accepted){
		qDebug() << "host inforation accepted";
	}
}

void MainWindow::on_ping_clicked() {
	order ord(order::e_type::PING);
	m_cmd_exec->sendNetRequest(ord);
}

// not used yed
void MainWindow::SavePeers(QString file_name) {
	ParamsContainer container;
	//container.setPeerList(m_peer_lst);
	container.writeParams(file_name);
}

void MainWindow::add_to_debug_window(const std::string &message) {
	ui->debugWidget->addItem(message.c_str());
	ui->debugWidget->scrollToBottom();
}

void MainWindow::show_peers(const std::vector<std::string> &peers) {
	ui->peerListWidget->clear();
	for (const auto &element : peers) {
		ui->peerListWidget->addItem(QString(element.c_str()));
	}
}

void MainWindow::on_actionDebug_triggered() {
	qDebug()<< "show dialog";
		DebugDialog dialog;
		dialog.exec();
		dialog.show();
}
