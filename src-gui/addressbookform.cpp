#include <QByteArray>
#include <QDebug>
#include <QFile>

#include "addressbookform.h"
#include "ui_addressbookform.h"

#include "meshpeer.h"

addressBookForm::addressBookForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::addressBookForm)
{
    ui->setupUi(this);
}

addressBookForm::~addressBookForm()
{
    delete ui;
}

void addressBookForm::loadPeersFromFiles(const QString &file_name)
{
    QFile file(file_name);
    if(!file.open(QFile::ReadOnly)) {
        qDebug()<<"can not load peers "<<file_name;
        return;
    }
    std::string data = file.readAll().toStdString();
    nlohmann::json json_list = nlohmann::json::parse(data);
    for (auto it :json_list) {
        if(it.at("obj")== "MeshPeer" ) {

        }
    }
}

void addressBookForm::savePeersToFile(const QString &file_name)
{
    QFile file(file_name);
    if(!file.open(QFile::ReadWrite)) {
        qDebug()<<"can't open file "<<file_name;
        return;
    }
}
