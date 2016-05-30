#include "mainwindow.h"
#include "ui_mainwindow.h"


#include "dialog.h"
#include <QDebug>
#include "mydelegate.h"
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{

	int num1 = 42;
	int num2 = 999;

	QString to_translate = tr(" this is msg  %1 with another number %2");						//string to translate
	QString myMsg = to_translate.arg(num1, 5, 10, QChar('0')).arg(num2, 5, 10, QChar('0'));		//preparing string

//	qDebug()<<to_translate;
//	QString myString = tr("to jest %d w ");

	qDebug()<< myMsg;
	ui->setupUi(this);

	dl =new myDelegate;

	for (int row = 0; row < 4; ++row) {
		for (int column = 0; column < 4; ++column) {
			QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
//			model.insert(row, column, item);
//			model.insertRow(column, item);
			model.setItem(column,row,item);

}
	}

	QList<QStandardItem *> lst;
	lst.insert(0,new QStandardItem (QString ("bla bla")));
	lst.insert(1,new QStandardItem (QString ("ble ble")));
	lst.insert(2,new QStandardItem (QString ("fefe")));
	//	model.setColumnCount(3);
//	lst.push_back(new QStandardItem (QString ("bla bla")));
//	lst.push_back(new QStandardItem (QString ("ble ble")));
//	lst.push_back(new QStandardItem (QString ("fefe")));

	for(int i=0;i<lst.size();++i){
		qDebug()<<lst.at(i);

	}

//		model.insertRow(0,lst);
	model.setColumnCount(4);
	ui->listView->setModel(&model);
	ui->listView->setItemDelegate(new myDelegate);
	ui->listView->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui->listView->setEditTriggers(QAbstractItemView::DoubleClicked
								   | QAbstractItemView::SelectedClicked);
//	ui->listView->setIte
}



MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButton_clicked()
{
	Dialog dlg;
	dlg.exec();
}
