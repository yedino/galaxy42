#include "debugdialog.hpp"
#include "ui_debugdialog.h"

#include <QTextDocument>

DebugDialog::DebugDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DebugDialog)
{
	ui->setupUi(this);
	QString ba ="<HEAD><TITLE>A Small Hello</TITLE> </HEAD><BODY><H1>Hi</H1><P>This is very minimal hello world HTML document.</P></BODY></HTML>";
	QTextDocument *docu = new QTextDocument(this);
	//docu->setHtml(QUrl("https://www.kernel.org").host());
	docu->setHtml(ba);

	ui->textEdit->setDocument(docu);

}

DebugDialog::~DebugDialog()
{
	delete ui;
}
