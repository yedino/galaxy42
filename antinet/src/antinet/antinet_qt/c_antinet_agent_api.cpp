#include "c_antinet_agent_api.hpp"
#include "ui_c_antinet_agent_api.h"

#include <limits>

using namespace std;

c_antinet_agent_api::c_antinet_agent_api(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::c_antinet_agent_api) {
    ui->setupUi(this);

    ui->emitValue->setValidator(new QIntValidator(0,numeric_limits<int>::max(),this));
    ui->sendValue->setValidator(new QIntValidator(0,numeric_limits<int>::max(),this));
    ui->receivedValue->setValidator(new QIntValidator(0,numeric_limits<int>::max(),this));

    QStringList *tokens = new QStringList();
    tokens->push_back("Tigutoken");
    tokens->push_back("Loltoken");

    ui->emitToken->addItems(*tokens);
    ui->sendToken->addItems(*tokens);
    tokens->clear();

    tokens->push_back("Othertoken1");
    tokens->push_back("Othertoken2");
    ui->receivedTokens->addItems(*tokens);
    delete tokens;

}

c_antinet_agent_api::~c_antinet_agent_api() {
    delete ui;
}

std::vector< c_antinet_agent_api_msg > c_antinet_agent_api::eat_messages() {
    std::vector< c_antinet_agent_api_msg > cpy;
    cpy.swap(m_messsages); // also erases here
    return cpy;
}

void c_antinet_agent_api::handle_exiting_now() {
    cout << "handle_exiting_now" << endl;
}

void c_antinet_agent_api::tokens_emit(const t_tok_id &token, const t_tok_amount &amount) {
    QString te_str = "Emit: " + QString::number(amount) + " - " + QString::fromStdString(token) +  " tokens";

    m_tok_emited[token] += amount;
    m_wallet[token] += amount;

    ui->lw_emited->clear();
    ui->lw_wallet->clear();
    for(std::map<t_tok_id,t_tok_amount>::iterator it=m_tok_emited.begin(); it!=m_tok_emited.end(); ++it) {
        ui->lw_emited->addItem(QString::fromStdString(it->first)+" - "+QString::number(it->second));
        //std::cout << it->first << " => " << it->second << '\n';
    }
    for(std::map<t_tok_id,t_tok_amount>::iterator it=m_wallet.begin(); it!=m_wallet.end(); ++it) {
        ui->lw_wallet->addItem(QString::fromStdString(it->first)+" - "+QString::number(it->second));
        //std::cout << it->first << " => " << it->second << '\n';
    }
    ui->te_msgView->append(te_str);
}

void c_antinet_agent_api::tokens_xfer(const t_tok_id &token, const t_tok_amount &amount, const t_nym_id recipient, const t_tok_id &for_token, const t_tok_amount &for_amount) {
    QString txf_str = 	"Tokens xfer: " + QString::number(amount) + ' ' + QString::fromStdString(token) + " to "
                        + QString::fromStdString(recipient) + " for " + QString::number(for_amount) + ' ' + QString::fromStdString(for_token);
    ui->te_msgView->append(txf_str);
}

void c_antinet_agent_api::on_buttonEmit_clicked() {
    t_tok_id choosenToken = ui->emitToken->currentText().toStdString();
    t_tok_amount choosenValue= ui->emitValue->text().toLong();
    tokens_emit(choosenToken,choosenValue);
    m_messsages.emplace_back(c_antinet_agent_api_msg("Succesfully emited tokens"));
}

void c_antinet_agent_api::on_buttonXfer_clicked() {
    t_tok_id choosenToken = ui->sendToken->currentText().toStdString();
    t_tok_amount choosenValue= ui->sendValue->text().toLong();
    t_tok_id receiverToken = ui->receivedTokens->currentText().toStdString();
    t_tok_amount receiverValue = ui->receivedValue->text().toLong();
    tokens_xfer(choosenToken,choosenValue,ui->receiver->text().toStdString(),receiverToken,receiverValue);
}
