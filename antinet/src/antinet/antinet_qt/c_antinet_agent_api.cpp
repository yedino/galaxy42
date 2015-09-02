#include "c_antinet_agent_api.h"
#include "ui_c_antinet_agent_api.h"

using namespace std;

c_antinet_agent_api::c_antinet_agent_api(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::c_antinet_agent_api)
{
    ui->setupUi(this);

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

c_antinet_agent_api::~c_antinet_agent_api()
{
    delete ui;
}


std::vector< c_antinet_agent_api_msg > c_antinet_agent_api::eat_messages() {
    std::vector< c_antinet_agent_api_msg > cpy;
    cpy.swap(m_messsages); // also erases here
    return cpy;
}

void c_antinet_agent_api::handle_exiting_now()
{
    cout << "handle_exiting_now" << endl;
}

void c_antinet_agent_api::tokens_emit(const t_tok_id &token, const t_tok_amount &amount)
{
    cout << "tokens_emit: " << token << ", " << amount << endl;
}

void c_antinet_agent_api::tokens_xfer(const t_tok_id &token, const t_tok_amount &amount, const t_nym_id recipient, const t_tok_id &for_token, const t_tok_amount &for_amount)
{
    cout << "tokens_xfer: " << token << ", " << amount << " to " << recipient << " for : " << for_token << ", " << for_amount << endl;
}

void c_antinet_agent_api::on_buttonEmit_clicked()
{
    t_tok_id choosenToken = ui->emitToken->currentText().toStdString();
    t_tok_amount choosenValue= ui->emitValue->text().toLong();
    tokens_emit(choosenToken,choosenValue);
    m_messsages.emplace_back(c_antinet_agent_api_msg("Succesfully emited tokens"));
}

void c_antinet_agent_api::on_buttonXfer_clicked()
{
    t_tok_id choosenToken = ui->emitToken->currentText().toStdString();
    t_tok_amount choosenValue= ui->emitValue->text().toLong();
    t_tok_id receiverToken = ui->receivedTokens->currentText().toStdString();
    t_tok_amount receiverValue = ui->receivedValue->text().toLong();
    tokens_xfer(choosenToken,choosenValue,ui->receiver->text().toStdString(),receiverToken,receiverValue);
}
