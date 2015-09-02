/********************************************************************************
** Form generated from reading UI file 'c_antinet_agent_api.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_C_ANTINET_AGENT_API_H
#define UI_C_ANTINET_AGENT_API_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

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
            c_antinet_agent_api->setObjectName(QStringLiteral("c_antinet_agent_api"));
        c_antinet_agent_api->resize(519, 478);
        centralWidget = new QWidget(c_antinet_agent_api);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 251, 121));
        emitValue = new QLineEdit(groupBox);
        emitValue->setObjectName(QStringLiteral("emitValue"));
        emitValue->setGeometry(QRect(10, 30, 113, 22));
        emitToken = new QComboBox(groupBox);
        emitToken->setObjectName(QStringLiteral("emitToken"));
        emitToken->setGeometry(QRect(130, 30, 101, 22));
        buttonEmit = new QPushButton(groupBox);
        buttonEmit->setObjectName(QStringLiteral("buttonEmit"));
        buttonEmit->setGeometry(QRect(130, 80, 101, 22));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 150, 481, 161));
        buttonXfer = new QPushButton(groupBox_2);
        buttonXfer->setObjectName(QStringLiteral("buttonXfer"));
        buttonXfer->setGeometry(QRect(360, 120, 101, 22));
        sendToken = new QComboBox(groupBox_2);
        sendToken->setObjectName(QStringLiteral("sendToken"));
        sendToken->setGeometry(QRect(190, 40, 72, 22));
        sendValue = new QLineEdit(groupBox_2);
        sendValue->setObjectName(QStringLiteral("sendValue"));
        sendValue->setGeometry(QRect(50, 40, 113, 22));
        label = new QLabel(groupBox_2);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 40, 31, 21));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(170, 40, 16, 21));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(270, 40, 16, 21));
        receiver = new QLineEdit(groupBox_2);
        receiver->setObjectName(QStringLiteral("receiver"));
        receiver->setGeometry(QRect(290, 40, 113, 22));
        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 100, 21, 21));
        receivedValue = new QLineEdit(groupBox_2);
        receivedValue->setObjectName(QStringLiteral("receivedValue"));
        receivedValue->setGeometry(QRect(50, 100, 113, 22));
        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(170, 100, 16, 21));
        receivedTokens = new QComboBox(groupBox_2);
        receivedTokens->setObjectName(QStringLiteral("receivedTokens"));
        receivedTokens->setGeometry(QRect(190, 100, 72, 22));
        msgView = new QTextEdit(centralWidget);
        msgView->setObjectName(QStringLiteral("msgView"));
        msgView->setGeometry(QRect(10, 320, 481, 91));
        c_antinet_agent_api->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(c_antinet_agent_api);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 519, 19));
        c_antinet_agent_api->setMenuBar(menuBar);
        mainToolBar = new QToolBar(c_antinet_agent_api);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        c_antinet_agent_api->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(c_antinet_agent_api);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        c_antinet_agent_api->setStatusBar(statusBar);

        retranslateUi(c_antinet_agent_api);

        QMetaObject::connectSlotsByName(c_antinet_agent_api);
    } // setupUi

    void retranslateUi(QMainWindow *c_antinet_agent_api)
    {
        c_antinet_agent_api->setWindowTitle(QApplication::translate("c_antinet_agent_api", "c_antinet_agent_api", 0));
        groupBox->setTitle(QApplication::translate("c_antinet_agent_api", "Emit tokens", 0));
        buttonEmit->setText(QApplication::translate("c_antinet_agent_api", "Emit", 0));
        groupBox_2->setTitle(QApplication::translate("c_antinet_agent_api", "Send", 0));
        buttonXfer->setText(QApplication::translate("c_antinet_agent_api", "Xfer", 0));
        label->setText(QApplication::translate("c_antinet_agent_api", "Send", 0));
        label_2->setText(QApplication::translate("c_antinet_agent_api", "of", 0));
        label_3->setText(QApplication::translate("c_antinet_agent_api", "to", 0));
        label_4->setText(QApplication::translate("c_antinet_agent_api", "for", 0));
        label_5->setText(QApplication::translate("c_antinet_agent_api", "of", 0));
    } // retranslateUi

};

namespace Ui {
    class c_antinet_agent_api: public Ui_c_antinet_agent_api {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_C_ANTINET_AGENT_API_H
