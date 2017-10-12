#ifndef RPCCOUNTER_H
#define RPCCOUNTER_H

#include <QObject>


class RpcCounter;

class RpcId {				//class wich is could be created from RpcCounter- to make sure the RpcIp is uniqe
public:
    std::string m_id;
protected:
    RpcId(QString id){m_id = id.toStdString();}
    RpcId(std::string id){m_id = id;}
    friend class RpcCounter;

};

class RpcCounter : public QObject
{
    Q_OBJECT
public:
    explicit RpcCounter(QString rpc_name,QObject *parent = nullptr);

    RpcId getRpcId()
    {
        if(m_name.isEmpty()){
            throw std::runtime_error ("no client name provided");
        }
        RpcId ret_val ( m_name + QString::number(m_counter));
        m_counter++  ;

        return ret_val;
    }
    void setRpcName(const QString &name);

signals:

public slots:

private:
    size_t m_counter = 0;
    QString m_name;
};

#endif // RPCCOUNTER_H
