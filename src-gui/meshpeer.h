#ifndef MESHPEER_H
#define MESHPEER_H

#include <QObject>
#include <json.hpp>

class MeshPeer : public QObject
{
    Q_OBJECT

    QString m_name;
    QString m_ip;
    QString m_vip;
    QString m_net_name;
    QString m_account;

    int m_to_pay;

    int m_ip_port;
    int m_vip_port;

public:

    enum class COMMANDSTATUS {
        none = 0,
        sended,			//send to node rpc
        recived,		//get conformation from rpc
        acknowled,		//state acknowled
        timeout,		//timeot of command
        fail			//failed to execute
    } comm_status;

    enum class STATUS {
        defalut = 0,	//unknown status -propably error
        connected,		//peer is connected
        disconnected,	//peer is disconnected
        stranger,		//peer is stranger
        known,			//peer is known()
        bfrend,			//peer is a friend
        banned			//peer is banned
    } status;

    enum class SOURCE {
        my = 0,			//created by me
        rpcExternal,	//get from external rpc client of this node(script may be)
        seed,			//get from seed
        inbound			// some stranger-tuneling by this node
    } source;


    explicit MeshPeer(QObject *parent = nullptr);

    explicit MeshPeer(const MeshPeer& peer);
    MeshPeer(const QString& serialized_obj, QObject *parent = nullptr);


    /**
     * @brief setName set user name
     * @param user name
     */
    inline void setName(const QString &name){m_name = name;}
    /**
     * @brief getName gets user name
     * @return user name
     */
    inline QString getName() const {return m_name;}

    /**
     * @brief getAccount return bitcoin account of user
     * @return account addres
     */
    inline QString getAccount() const {return m_account;}

    /**
     * @brief setIP check and sets ip of user - if improper throw exception
     * @param ip new ip
     */
    inline void setIP(const QString& ip)
    {
        //@todo dorobic sprawdzenie prawidlowosci ip
        m_ip = ip;
    }

    /**
     * @brief getIp return ip of user
     * @return ip
     */
    inline QString getIp() const {return m_ip;}

    /**
     * @brief setVip check and set virtual ip of user if improper throw exception
     * @param vip virtual ip
     */
    inline void setVip(const QString& vip)
    {
        //@todo check if vip is proper
        m_vip = vip;
    }

    /**
     * @brief getVip return virtual ip of user
     * @return
     */
    inline QString getVip()const{return m_vip;}

    /**
     * @brief getInvitation create invitation based on ip and vip of user
     * @return invitation
     */
    inline QString getInvitation() const
    {
        QString ret_val;
        ret_val= m_vip+"@";
        ret_val+=m_ip;
        return ret_val;
    }

    /**
     * @brief setInvitation reading ip and vip basedon invitation
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

    inline int getPort() const{return m_ip_port;}
    inline int getVipPort() const {return m_vip_port;}
    inline void setPort(int port){m_ip_port = port;}
    inline void setVipPort(int port){m_vip_port = port;}

    void setToPay(uint32_t satoshi);
    uint32_t getToPay() const{return m_to_pay;}

    void deserialize(const nlohmann::json &serialized_obj);
};

#endif // MESHPEER_H
