#include "addressdialog.hpp"
#include "ui_addressdialog.h"

addressDialog::addressDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::addressDialog)
{
	ui->setupUi(this);
}

addressDialog::~addressDialog()
{
	delete ui;
}

void addressDialog::on_buttonBox_accepted()
{
	emit add_address(ui->lineEdit->text());
}
