#include <json.hpp>
#include "meshpeer.h"

MeshPeer::MeshPeer( QObject *parent ) : QObject( parent )
{
    status = STATUS::disconnected;
}


MeshPeer::MeshPeer( const MeshPeer &peer ):m_name( peer.getName() ),m_ip( peer.getIp() ),m_vip( peer.getVip() ),
    comm_status( peer.comm_status ), status( peer.status ),source( source ),QObject( peer.parent() )
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
    j["obj"] = "MeshPeer";
    j["ver"] =	"0.01";
    j["name"] = getName().toStdString();
    j["ip"] = getIp().toStdString();
    j["vip"] = getVip().toStdString();
    return j.dump();
}

void MeshPeer::deserialize( const std::string &serilized_obj )
{
    nlohmann::json j = nlohmann::json::parse( serilized_obj );


    std::string obj_name = j.at( "obj" ).get<std::string>();
    if( obj_name != "MeshPeer" ) {
        throw std::runtime_error ( tr( "can't deserialize error" ).toStdString() );
    }

    m_name = QString::fromStdString( j.at( "name" ).get<std::string>() );
    m_ip = QString::fromStdString( j.at( "ip" ).get<std::string>() );
    m_vip = QString::fromStdString( j.at( "vip" ).get<std::string>() );

    return;
}
