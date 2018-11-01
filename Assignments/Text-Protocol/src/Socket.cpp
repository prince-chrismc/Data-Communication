/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "Socket.h"
#include <stdexcept>

TextProtocol::Socket::Socket() : CActiveSocket( SocketTypeUdp ), m_Expected{ 0 }, m_Requested{ 0 },
m_ServerIp{ 0 }, m_ServerPort{ 0 }
{
}

bool TextProtocol::Socket::Open( const char * pAddr, uint16 nPort )
{
   const auto retval = CActiveSocket::Open( pAddr, nPort ); // TO DO: Swap with routeres information

   m_ServerIp = IpV4Address{ m_stServerSockaddr.sin_addr.s_addr };
   m_ServerPort = PortNumber{ m_stServerSockaddr.sin_port };

   return retval;
}

bool TextProtocol::Socket::Send( const Message& toSend )
{
   std::string msgPayload = toSend.ToByteStream();

   if( msgPayload.length() < 12 ) throw std::runtime_error( "no point in sending an incomplete message" );

   const auto bytesSent = CActiveSocket::Send( reinterpret_cast<const uint8*>( &msgPayload.front() ),
                                               toSend.Size() );

   return ( static_cast<size_t>(bytesSent) == msgPayload.length() );
}

TextProtocol::Message TextProtocol::Socket::Receive()
{
   CActiveSocket::Receive( Message::MAX_MESSAGE_SIZE );
   std::string bytesRx = reinterpret_cast<const char*>( GetData() );

   return Message::Parse( bytesRx );
}
