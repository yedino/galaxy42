#include "dialog.h"
#include "ui_dialog.h"
#include <QGraphicsWidget>
#include <QImageReader>
#include <QStringList>
#include <QStandardItemModel>
#include <QTcpServer>
#include <QDebug>
#include <QTcpSocket>



/**
 * @brief Dialog::Dialog
 * @param parent
 * @todo  wyciagnac do osobnego programu
 * 	obsulzyc wysylanie odpowiedzi
 * 	poprawic grafike okienek logu
 * 	dodac kolorki
 * 	ikonka - dress
 *
 */
Dialog::Dialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dialog)
{
	connect(&m_server,SIGNAL(newConnection()),this,SLOT(onConnect()) );

	if(!m_server.listen(QHostAddress::LocalHost,9040)) {
		qDebug()<<"can't start server";
	}

	ui->setupUi(this);
	QIcon icon(":/qrCode.png");

	ui->qrPushButton_4->setIcon(icon);
	ui->qrPushButton_4->setIconSize(QSize(90,90));

	QStringList l_elist;

	l_elist.push_back("warrning: pierwsze ostrzezenie;");
	l_elist.push_back("error: drugie ostrzezenie;");
	l_elist.push_back("warrning:trzecie ostrzezenie;");

	m_info_model = new infoModel(l_elist,this);

	ui->errorView->setModel(m_info_model);
	ui->questionView->setModel(m_info_model);
	ui->errorView->resizeColumnsToContents();
	m_info_model->addMessage("question:czwarte ostrzezenie no;");

//	ui->errorView->setContextMenuPolicy(Qt::);
	ui->errorView->resizeColumnsToContents();
	ui->questionView->resizeColumnsToContents();


//	l_elist.push_back("warrning: nowe ostrzezenie");
	m_info_model->addMessage("warrning: nowe ostrzezenie");

}

Dialog::~Dialog()
{
	delete ui;
}

void Dialog::on_qrPushButton_4_clicked()
{
	QDialog l_smallDialog;
	 QImageReader reader(":/qrCode.png");
	QImage ll = reader.read();

	QLabel l_label(&l_smallDialog);
	l_label.setPixmap(QPixmap (":/qrCode.png"));

	l_smallDialog.setMinimumSize(150,150);
	l_smallDialog.exec();

}

void Dialog::onRead()
{
		QByteArray msg = m_tmp_socket->readAll();
		m_tmp_socket->write("ACK");
		m_tmp_socket->flush();
		m_tmp_socket->waitForBytesWritten();
	   m_tmp_socket->close();
		m_info_model->addMessage(QString(msg));

		ui->errorView->selectRow(m_info_model->rowCount()-1);
		ui->questionView->selectRow(m_info_model->rowCount()-1);

}

void Dialog::write(QByteArray)
{

}

void Dialog::onConnect()
{
	m_tmp_socket = m_server.nextPendingConnection();
	connect(m_tmp_socket,SIGNAL(readyRead()),this,SLOT(onRead()));

//	connect()

//       socket->write("Hello client\r\n");
//   socket->flush();

//       socket->waitForBytesWritten(3000);

}

