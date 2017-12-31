#include <QDebug>

#include <json.hpp>
#include "meshpeer.h"

MeshPeer::MeshPeer( QObject *parent ) : QObject( parent )
{
    status = STATUS::disconnected;
}


MeshPeer::MeshPeer( const MeshPeer &peer ):m_name( peer.getName() ),m_ip( peer.getIp() ),m_vip( peer.getVip() ),
    comm_status( peer.comm_status ), status( peer.status ),source( peer.source ),QObject( peer.parent() )
{
    ;
}


MeshPeer::MeshPeer( const QString &serialized_obj, QObject *parent ):QObject( parent )
{
    status = STATUS::disconnected;
    deserialize( serialized_obj.toStdString() );
}

std::string MeshPeer::serialize() const
{
    nlohmann::json j;
    try{
        j["obj"] = "MeshPeer";
        j["ver"] =	"0.01";
        j["name"] = getName().toStdString();
        j["ip"] = getIp().toStdString();
        j["vip"] = getVip().toStdString();
        j["ip_port"] = getPort();
        j["vip_port"] = getVipPort();
        j["source"] = static_cast<int>(source);
    } catch ( std::exception &e ) {
        qDebug()<<"problem while serialize"<<e.what();
    }

    return j.dump();
}

void MeshPeer::deserialize(const nlohmann::json & serialized_obj)
{

    std::string obj_name = serialized_obj.at( "obj" ).get<std::string>();
    if( obj_name != "MeshPeer" ) {
        throw std::runtime_error ( tr( "can't deserialize error" ).toStdString() );
    }

    try {
        m_name = QString::fromStdString( serialized_obj.at( "name" ).get<std::string>() );
        m_ip = QString::fromStdString( serialized_obj.at( "ip" ).get<std::string>() );
        m_vip = QString::fromStdString( serialized_obj.at( "vip" ).get<std::string>() );
        m_ip_port = serialized_obj.at("ip_port").get<int>();
        m_vip_port = serialized_obj.at("vip_port").get<int>();
        source = static_cast<MeshPeer::SOURCE>(serialized_obj.at("source").get<int>());
    } catch ( std::exception &e ) {
        qDebug()<<"problem while deserialize"<< e.what();
    }
    return;
}

void MeshPeer::deserialize( const std::string &serilized_obj )
{
    nlohmann::json j = nlohmann::json::parse( serilized_obj );
    deserialize(j);
/*
    std::string obj_name = j.at( "obj" ).get<std::string>();
    if( obj_name != "MeshPeer" ) {
        throw std::runtime_error ( tr( "can't deserialize error" ).toStdString() );
    }

    try{
        m_name = QString::fromStdString( j.at( "name" ).get<std::string>() );
        m_ip = QString::fromStdString( j.at( "ip" ).get<std::string>() );
        m_vip = QString::fromStdString( j.at( "vip" ).get<std::string>() );
        m_ip_port = j.at("ip_port").get<int>();
        m_vip_port = j.at("vip_port").get<int>();
        source = static_cast<MeshPeer::SOURCE>(j.at("source").get<int>());
    } catch ( std::exception &e ) {
        qDebug()<<"problem while deserialize"<< e.what();
    }
    return;
    */
}
