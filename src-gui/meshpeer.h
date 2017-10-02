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
        sended,			//wyslano do noda galaxy
        recived,		//otrzymano potwierdzenie z sieci galaxy
        acknowled,		//zsynchronizowano w sieci galaxy
        timeout,		// timeout komendy
        fail			//nieudane wykonnie komendy na no
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
     * @brief setName ustawia nazwe urzytkownika
     * @param name imie
     */
    inline void setName(const QString &name){m_name = name;}
    /**
     * @brief getName pobiera nazwe
     * @return nazwa uzytkownika
     */
    inline QString getName() const {return m_name;}

    /**
     * @brief setIP sprawdza i ustawia dane ip, w  razie niepowodzenia wyrzuca std::runtime_exception
     * @param ip nowe ip
     */
    inline void setIP(const QString& ip)
    {
        //@todo dorobic sprawdzenie prawidlowosci ip
        m_ip = ip;
    }

    /**
     * @brief getIp zwraca ip
     * @return
     */
    inline QString getIp() const {return m_ip;}

    /**
     * @brief setVip sprawdza virtual ip i ustawia go w klasie- w razie niepowodzenia - wyrzuca wyjatek
     * @param vip virtual ip danego peera
     */
    inline void setVip(const QString& vip)
    {
        //@todo dorobic sprawdzenie czy prawidlowy vip
        m_vip = vip;
    }

    /**
     * @brief getVip zwraca virtual ip danego peera
     * @return
     */
    inline QString getVip()const{return m_vip;}

    /**
     * @brief getInvitation sklada invitation- na podstawie wlasnego vip i ip
     * @return nr zaproszenia
     */
    inline QString getInvitation() const
    {
        QString ret_val;
        ret_val= m_vip+"@";
        ret_val+=m_ip;
        return ret_val;
    }

    /**
     * @brief setInvitation ustawia wlasciwosci obiektu na podstwie numeru zaproszenia
     * @param invitation zaproszenie
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
