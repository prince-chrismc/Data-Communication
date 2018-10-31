// insert license

#include "Socket.h"

bool TextProtocol::Socket::Open( const char * pAddr, uint16 nPort )
{
   CActiveSocket::Open( pAddr, nPort );

   m_ServerIp = IpV4Address{ m_stServerSockaddr.sin_addr.s_addr };


   return false;
}
