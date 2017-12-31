#include "rpccounter.h"

RpcCounter::RpcCounter( QString rpc_name,QObject *parent ) : QObject( parent ), m_name( rpc_name )
{
    m_counter = 100;
}


void RpcCounter::setRpcName( const QString &name )
{
    if( m_name != name ) {
        m_name = name;
        m_counter = 100;
    }
}
