#ifndef SENDEDCOMMAND_H
#define SENDEDCOMMAND_H

#include <memory>

#include <QObject>
#include <QMap>


#include "order.hpp"

class order;

class SendedCommands : public QObject
{
    Q_OBJECT
public:
    SendedCommands(QObject *parent = nullptr);

    void addOrder(std::shared_ptr<order> added_order,const QString &rpc_num)
    {
        if(rpc_num.size() == 0){
            return;
        }

        if( m_orders.contains(rpc_num) ){
            throw std::runtime_error("rpc num allrady exist");
        }
        m_orders.insert(rpc_num,added_order);
    }

    std::shared_ptr<order> getOrder(const QString& rpc_num)
    {
        if( !m_orders.contains(rpc_num) ) {
            throw std::runtime_error("can't find this order  - did I send it?");
        }

        std::shared_ptr<order> ret_val = m_orders.value(rpc_num);
        m_orders.remove(rpc_num);
        return ret_val;
    }

    void clearOrders()
    {
        m_orders.clear();
    }

    size_t ordersCount()
    {
        return m_orders.size();
    }

private:
    QMap<QString,std::shared_ptr<order> > m_orders;

};

#endif // SENDEDCOMMAND_H
