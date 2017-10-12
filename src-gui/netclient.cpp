#include "netclient.hpp"
#include <cassert>
#include <memory>

netClient::netClient( commandExecutor* cmd_exec_ptr )
    :
    m_cmd_exec( cmd_exec_ptr ),
    m_socket( std::make_unique<QTcpSocket>() ),
    m_hmac_key()
{
    connect( m_socket.get(), SIGNAL( readyRead() ),this, SLOT( onTcpReceive() ) );
    m_hmac_key.fill( 0x42 ); // @todo load from settings!!!
}


bool netClient::startConnect( const QHostAddress &address, uint16_t port ) {
    m_socket->connectToHost( address, port );
    // we need to wait...
    if( !m_socket->waitForConnected( 5000 ) ) {
        qDebug() << "Error: " << m_socket->errorString();
        return false;
    }
    return true;
}

void netClient::closeConnection()
{
    m_socket->close();
}

bool netClient::is_connected() {
    if( m_socket == nullptr ) {
        qDebug()<<"Socket is not defined (nullptr)";
        return false;
    }
    else if ( m_socket->state() != QAbstractSocket::ConnectedState ) {
        qDebug()<<"Socket is not connected";
        return false;
    } else {
        return true;
    }
}

void netClient::send_msg( const std::string &msg ) {
    if ( !is_connected() ) return;
    qDebug()<<"send:"<<msg.c_str();
    QByteArray packet = serialize_msg( msg );
    size_t send_bytes = static_cast<size_t>( m_socket->write( packet ) );
    if ( send_bytes != static_cast<size_t>( packet.size() ) )
        throw std::runtime_error( "send packet error" );
    qDebug() << "authenticate message";
    QByteArray authenticator( crypto_auth_hmacsha512_BYTES, 0 );
    int ret = crypto_auth_hmacsha512(
                  reinterpret_cast<unsigned char *>( authenticator.data() ),
                  reinterpret_cast<const unsigned char *>( msg.data() ),
                  msg.size(),
                  m_hmac_key.data() );
    if ( ret == -1 )
        throw std::runtime_error( tr( "authentication error" ).toStdString() );
    assert( ret == 0 );
    if ( authenticator.size() != m_socket->write( authenticator ) )
        throw std::runtime_error( tr( "send authenticator error" ).toStdString() );
}

void netClient::onTcpReceive() {
    qDebug() << "onTcpReceive\n";
    QByteArray data_array = m_socket->readAll();
    std::string arr( data_array.data(), static_cast<size_t>( data_array.size() ) );
    m_data_eater.eat( arr );
    std::string last_cmd = m_data_eater.getLastCommand(); // last cmd == cmd + authenticator
    if ( !last_cmd.empty() ) {
        if ( !check_auth( last_cmd ) ) {
            qDebug() << "message auth error, drop connection";
            m_socket->abort();
            return;
        }
        last_cmd.erase( last_cmd.end() - crypto_auth_hmacsha512_BYTES, last_cmd.end() );
        qDebug() << "last command " << QString::fromStdString( last_cmd );
        auto cmd_exec_ptr = m_cmd_exec;//.lock();
        cmd_exec_ptr->parseAndExecMsg( last_cmd );
    }
}

QByteArray netClient::serialize_msg( const std::string &msg ) {
    assert( msg.size() <= std::numeric_limits<uint16_t>::max() && "Too big message" );
    uint16_t msg_size = static_cast<uint16_t>( msg.size() );

    QByteArray packet( msg_size + 2, 0 ); // 2 is bytes for size

    packet[0] = static_cast<char>( msg_size >> 8 );
    packet[1] = static_cast<char>( msg_size & 0xFF );

    for ( unsigned int i = 0; i < msg_size; ++i ) {
        packet[i + 2] = msg.at( i );
    }
    return packet;
}

bool netClient::check_auth( const std::string &cmd_and_auth ) {
    if ( cmd_and_auth.size() <= crypto_auth_hmacsha512_BYTES ) return false;
    size_t cmd_size = cmd_and_auth.size() - crypto_auth_hmacsha512_BYTES;
    const unsigned char *hash = reinterpret_cast<const unsigned char *>( cmd_and_auth.data() ) + cmd_size;
    int ret = crypto_auth_hmacsha512_verify( hash,
              reinterpret_cast<const unsigned char *>( cmd_and_auth.data() ),
              cmd_size,
              m_hmac_key.data() );
    return ( ret == 0 );
}
