#ifndef MESHPEER_H
#define MESHPEER_H

#include <QObject>

class MeshPeer : public QObject
{
    Q_OBJECT

    QString m_name;
    QString m_ip;
    QString m_vip;
    QString m_net_name;


public:


    enum COMMANDSTATUS {
        none = 0,
        sended,
        recived,
        acknowled,
        timeout,
        fail
    } comm_status;


    enum STATUS {
        defalut = 0,
        connected,
        disconnected,
        stranger,
        known,
        bfrend,
        banned
    } status;

    enum SOURCE {
        my = 0,
        rpcExternal,
        seed,
        inbound
    } source;


    explicit MeshPeer(QObject *parent = nullptr);
    explicit MeshPeer(const MeshPeer& peer);
    MeshPeer(const QString& serialized_obj, QObject *parent = nullptr);


    /**
     * @brief setName set name of user
     * @param name imie
     */
    inline void setName(const QString &name){m_name = name;}
    /**
     * @brief getName geting name of user
     * @return user name
     */
    inline QString getName() const {return m_name;}

    /**
     * @brief setIP test and set ip if unproper throw std::runtime_exception
     * @param ip new ip
     */
    inline void setIP(const QString& ip)
    {
        //@todo dorobic sprawdzenie prawidlowosci ip
        m_ip = ip;
    }

    /**
     * @brief getIp return ip
     * @return
     */
    inline QString getIp() const {return m_ip;}

    /**
     * @brief setVip try and set virtual vip(ipv6) if fail throw std::runtime exception
     * @param vip virtual ip danego peera
     */
    inline void setVip(const QString& vip)
    {
        //!@todo test if ip is proper
        m_vip = vip;
    }

    /**
     * @brief getVip return vip of peer
     * @return
     */
    inline QString getVip()const{return m_vip;}

    /**
     * @brief getInvitation prepare invitation
     * @return inbitation
     */
    inline QString getInvitation() const
    {
        QString ret_val;
        ret_val= m_vip+"@";
        ret_val+=m_ip;
        return ret_val;
    }

    /**
     * @brief setInvitation sets params of object via invitation
     * @param invitation
     */
    inline void setInvitation(const QString& invitation)
    {
        QStringList list = invitation.split("@");
        if(list.size()!= 2) {
            throw std::runtime_error ("improper no of arguments in invitation");
        }
        setVip(list.at(0));
        setIP(list.at(1));
    }

    std::string serialize() const;
    void deserialize(const std::string& serilized_obj);

};

#endif // MESHPEER_H
