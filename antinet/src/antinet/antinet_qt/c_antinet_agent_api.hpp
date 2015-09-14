#ifndef C_ANTINET_AGENT_API_HPP
#define C_ANTINET_AGENT_API_HPP

#include <QMainWindow>
#include <iostream>
#include <map>

#include "c_antinet_agent_api_msg.hpp"

typedef long int t_tok_amount;
typedef std::string t_tok_id;
typedef std::string t_nym_id;

namespace Ui {
class c_antinet_agent_api;
}

class c_antinet_agent_api : public QMainWindow {
    Q_OBJECT

  public:
    explicit c_antinet_agent_api(QWidget *parent = 0);
    ~c_antinet_agent_api();

    /**
        * call this when we are about to exit. this can process all pending events (but does not have to), and will call a tick() itself if needed
        */
    void handle_exiting_now();
    std::vector< c_antinet_agent_api_msg > eat_messages();
    void tokens_emit(const t_tok_id &token, const t_tok_amount &amount);
    void tokens_xfer(const t_tok_id &token, const t_tok_amount &amount, const t_nym_id recipient, const t_tok_id &for_token, const t_tok_amount &for_amount);

    std::vector< c_antinet_agent_api_msg > m_messsages;
    std::map<t_tok_id,t_tok_amount> m_wallet;
    std::map<t_tok_id,t_tok_amount> m_tok_emited;

  private slots:
    void on_buttonEmit_clicked();
    void on_buttonXfer_clicked();

  private:
    Ui::c_antinet_agent_api *ui;
};

#endif // C_ANTINET_AGENT_API_HPP


