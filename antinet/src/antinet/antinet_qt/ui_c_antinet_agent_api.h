/********************************************************************************
** Form generated from reading UI file 'c_antinet_agent_api.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_C_ANTINET_AGENT_API_H
#define UI_C_ANTINET_AGENT_API_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_c_antinet_agent_api
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QLineEdit *emitValue;
    QComboBox *emitToken;
    QPushButton *buttonEmit;
    QGroupBox *groupBox_2;
    QPushButton *buttonXfer;
    QComboBox *sendToken;
    QLineEdit *sendValue;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *receiver;
    QLabel *label_4;
    QLineEdit *receivedValue;
    QLabel *label_5;
    QComboBox *receivedTokens;
    QTextEdit *msgView;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *c_antinet_agent_api)
    {
        if (c_antinet_agent_api->objectName().isEmpty())
            c_antinet_agent_api->setObjectName(QString::fromUtf8("c_antinet_agent_api"));
        c_antinet_agent_api->resize(519, 478);
        centralWidget = new QWidget(c_antinet_agent_api);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 251, 121));
        emitValue = new QLineEdit(groupBox);
        emitValue->setObjectName(QString::fromUtf8("emitValue"));
        emitValue->setGeometry(QRect(10, 30, 113, 22));
        emitToken = new QComboBox(groupBox);
        emitToken->setObjectName(QString::fromUtf8("emitToken"));
        emitToken->setGeometry(QRect(130, 30, 101, 22));
        buttonEmit = new QPushButton(groupBox);
        buttonEmit->setObjectName(QString::fromUtf8("buttonEmit"));
        buttonEmit->setGeometry(QRect(130, 80, 101, 22));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 150, 481, 161));
        buttonXfer = new QPushButton(groupBox_2);
        buttonXfer->setObjectName(QString::fromUtf8("buttonXfer"));
        buttonXfer->setGeometry(QRect(360, 120, 101, 22));
        sendToken = new QComboBox(groupBox_2);
        sendToken->setObjectName(QString::fromUtf8("sendToken"));
        sendToken->setGeometry(QRect(190, 40, 72, 22));
        sendValue = new QLineEdit(groupBox_2);
        sendValue->setObjectName(QString::fromUtf8("sendValue"));
        sendValue->setGeometry(QRect(50, 40, 113, 22));
        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 40, 31, 21));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(170, 40, 16, 21));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(270, 40, 16, 21));
        receiver = new QLineEdit(groupBox_2);
        receiver->setObjectName(QString::fromUtf8("receiver"));
        receiver->setGeometry(QRect(290, 40, 113, 22));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 100, 21, 21));
        receivedValue = new QLineEdit(groupBox_2);
        receivedValue->setObjectName(QString::fromUtf8("receivedValue"));
        receivedValue->setGeometry(QRect(50, 100, 113, 22));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(170, 100, 16, 21));
        receivedTokens = new QComboBox(groupBox_2);
        receivedTokens->setObjectName(QString::fromUtf8("receivedTokens"));
        receivedTokens->setGeometry(QRect(190, 100, 72, 22));
        msgView = new QTextEdit(centralWidget);
        msgView->setObjectName(QString::fromUtf8("msgView"));
        msgView->setGeometry(QRect(10, 320, 481, 91));
        c_antinet_agent_api->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(c_antinet_agent_api);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 519, 19));
        c_antinet_agent_api->setMenuBar(menuBar);
        mainToolBar = new QToolBar(c_antinet_agent_api);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        c_antinet_agent_api->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(c_antinet_agent_api);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        c_antinet_agent_api->setStatusBar(statusBar);

        retranslateUi(c_antinet_agent_api);

        QMetaObject::connectSlotsByName(c_antinet_agent_api);
    } // setupUi

    void retranslateUi(QMainWindow *c_antinet_agent_api)
    {
        c_antinet_agent_api->setWindowTitle(QApplication::translate("c_antinet_agent_api", "c_antinet_agent_api", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("c_antinet_agent_api", "Emit tokens", 0, QApplication::UnicodeUTF8));
        buttonEmit->setText(QApplication::translate("c_antinet_agent_api", "Emit", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("c_antinet_agent_api", "Send", 0, QApplication::UnicodeUTF8));
        buttonXfer->setText(QApplication::translate("c_antinet_agent_api", "Xfer", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("c_antinet_agent_api", "Send", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("c_antinet_agent_api", "of", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("c_antinet_agent_api", "to", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("c_antinet_agent_api", "for", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("c_antinet_agent_api", "of", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class c_antinet_agent_api: public Ui_c_antinet_agent_api {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_C_ANTINET_AGENT_API_H
