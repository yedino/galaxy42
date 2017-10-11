#include "paramscontainer.hpp"

#include <QFile>
#include <QDebug>

ParamsContainer::ParamsContainer( QObject *parent ) : QObject( parent )
{

}

bool ParamsContainer::readParams( QString file_name )
{
    QFile file( file_name );
    m_was_readed = false;
    if ( !file.open( QFile::ReadOnly ) ) {
        return false;
    }
    QByteArray data = file.readAll();
    file.close();

    return true;
}

bool ParamsContainer::writeParams( QString file_name )
{
    QFile file( file_name );
    if ( !file.open( QFile::ReadOnly ) ) {
        return false;
    }
    return true;
}

QString ParamsContainer::getIp() {
    return m_my_ip;
}
std::vector<peer_reference> ParamsContainer::getPeerList() {
    return m_peers;
}

void ParamsContainer::setIp( QString my_ip ) {
    m_my_ip = my_ip;
}
void ParamsContainer::setPeerList( std::vector<peer_reference> &peer_list ) {
    m_peers = peer_list;
}
