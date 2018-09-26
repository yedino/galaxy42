#ifndef ORDER_HPP
#define ORDER_HPP

#include<stdint.h>

#include <json.hpp>
#include "meshpeer.h"
#include "commandsender.h"
#include "rpccounter.h"


class MainWindow;
class commandExecutor;
class NodeControlerDialog;

class order {
public:
    enum class e_type {
            PING = 0,
            PEER_LIST,
            ADD_PEER,
            BAN_PEER,
            BAN_ALL_PEER,
            DELETE_PEER,
            DELETE_ALL_PEERS,
            GET_INVITATION,
            SETIPS,
            None
    };

    order()
    {

    }

    order(const RpcId& Id)
    {
        m_id = Id.m_id;
        m_state = "ok";
    }

    order(const std::string &json_str);

    order(e_type cmd);

    inline std::string getId()const{return m_id;}

    order(order* ord)
    {
        m_cmd = 	ord->m_cmd;
        m_msg = 	ord->m_msg;
        m_command =	ord-> m_command;
        m_state =	ord->m_state;
        m_id =  	ord->m_id;
        m_rpc_name = ord->m_rpc_name;
    }

    virtual std::string get_str() const;


    std::string get_cmd() const;
    std::string get_msg() const;
    std::string get_params(std::string param_name) const;
    std::vector<std::string> get_msg_array() const;

    virtual void execute(MainWindow &main_window) = 0;
    virtual ~order(){;}


protected:
    std::string m_cmd;
    std::string m_msg;
    std::string m_command;
    std::string m_state;
    std::string m_id;
    std::vector<std::string> m_msg_array;
    std::string m_rpc_name;
    std::string m_re;
    commandExecutor *m_executor = nullptr;
    std::string m_peer;

    std::string getPeerName();
};


class basicOrder final:public order
{
public:

    basicOrder(const std::string &json_str);
    void execute(MainWindow &){return;}
protected :


};


class pingOrder final: public order
{
public:
    pingOrder(const RpcId& Id);

    pingOrder(pingOrder* ping)
    {
        m_msg = ping->m_msg;
        m_command = ping->m_command;
        m_id = ping->m_id;
        m_cmd = ping->m_cmd;
        m_msg_array = ping->m_msg_array;
    }

    pingOrder(const std::string &json_str);
    void execute(MainWindow &);
};

class peerListOrder final: public order
{
public:

    peerListOrder(const RpcId& Id);
    peerListOrder(const std::string &json_str);
    void execute(MainWindow &main_window);

protected:

};

class getGalaxyOrder final: public order
{


public:
    getGalaxyOrder(getGalaxyOrder *ord):order(ord){m_ipv6 = ord->m_ipv6;}

    getGalaxyOrder(const RpcId& Id,const std::vector<std::string> &ipv4_list);
    getGalaxyOrder(const std::string &json_str);
    virtual void execute(MainWindow &main_window);
    virtual std::string get_str() const;

protected:
    std::string m_ipv6;

};

class addPeerOrder final: public order
{
public:
    addPeerOrder(addPeerOrder *ord):order(ord){m_format = ord->m_format; m_peer = ord->m_peer;}
    addPeerOrder(const RpcId& id,const MeshPeer &peer);
    addPeerOrder(const std::string &json_str,commandExecutor *executor);
    virtual std::string get_str() const;
    void execute(MainWindow &main_window) override;

protected:
//    MeshPeer m_peer_obj;
    std::string m_format;
};

class banAllOrder final: public order
{
public:
    banAllOrder(banAllOrder* ord):order(ord){;}
    banAllOrder(const RpcId& Id);
    banAllOrder(const std::string &json_str);
    void execute(MainWindow &main_window);
    std::string get_str() const;

protected:

};


class deletePeerOrder final:public order
{
public:
    deletePeerOrder (deletePeerOrder *ord):order(ord){;}
    deletePeerOrder (const RpcId& Id,const MeshPeer &peer);

    deletePeerOrder(const std::string &json_str,commandExecutor *executor);
    void execute(MainWindow &main_window);
    virtual std::string get_str() const;
protected:
    std::string m_peer;
};

class deleteAllPeersOrder final: public order
{
public:
    deleteAllPeersOrder(const RpcId& Id);
    deleteAllPeersOrder(deleteAllPeersOrder* ord):order(ord){;}
    deleteAllPeersOrder(const std::string &json_str):order(json_str)
    {

    }

    void execute(MainWindow &main_window);
    std::string get_str() const;
protected:

};

class setIps final:public order
{
public:
    setIps(setIps * ord):order(ord)
    {
        m_ip_list = ord->m_ip_list;
    }
    setIps(const RpcId& Id,const std::vector<std::string > &ip_list);
    setIps(const std::string &json_str);
    std::string get_str() const;
    void execute(MainWindow &main_window);

protected:
    std::vector<std::string> m_ip_list;

};

class banPeerOrder final:public order
{
public :
    banPeerOrder(banPeerOrder* ord):order(ord)
    {
        m_peer = ord->m_peer;
    }

    banPeerOrder(const RpcId& Id,const MeshPeer &peer);
    banPeerOrder(const std::string &json_str,commandExecutor* exec);
    std::string get_str() const;
    void execute(MainWindow &main_window);
protected:

};

class getClientName final : public order
{
public:
    getClientName()
    {
        m_cmd ="hello";
        m_state = "ok";
    }
    getClientName(getClientName * ord):order(ord)
    {
        m_rpc_name ="hello";
    }

    getClientName(const std::string &json_str,commandExecutor *executor);
    std::string get_str() const;
    void execute(MainWindow &main_window);


protected:
    commandExecutor* m_executor = nullptr;
    std::string m_account;
};


class serverMsg final:public order
{
public:
    serverMsg(const std::string &json_str);
    void execute(MainWindow &main_window);

protected:
    std::string m_lvl;
private:
    std::string get_str() const{return std::string();}
};


class setAccountOrder final:public order
{
public:
    setAccountOrder(const RpcId& id,const MeshPeer &peer );
    setAccountOrder(const std::string &json_str,commandExecutor *executor);
    void execute(MainWindow &main_window);
    std::string get_str() const ;
private:
    std::string m_account;
};


class payOrder final :public order
{
public:
    payOrder(const RpcId& id,const MeshPeer &peer ,int satoshi =0);
    payOrder(const std::string &json_str,commandExecutor *executor);
    void execute(MainWindow &main_window);
    std::string get_str() const ;
private:

    uint32_t m_satoshi;

};


class statusOrder final:public order
{
public:

    statusOrder(const RpcId& Id);


    statusOrder(const std::string &json_str,commandExecutor *executor);
    void execute(MainWindow &main_window);

    std::string get_str() const;

private:

    int32_t m_satoshi;
};


class getGalaxyIpV6Order final:public order
{
public:
    getGalaxyIpV6Order(const RpcId& Id);
    getGalaxyIpV6Order(const std::string &json_str);
    void execute(MainWindow &main_window);
private:
    std::string m_ipv6;
};

#endif // ORDER_HPP
