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
#include <iostream>

TextProtocol::Socket::Socket() : CActiveSocket( SocketTypeUdp ), m_Expected{ 0 }, m_Requested{ 0 },
m_ServerIp{ 0 }, m_ServerPort{ 0 }
{
   if( ! Initialize() )
      throw std::runtime_error( "The OS was unable to create a socket!" );
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
   std::cout << "Socket::Send >> ";
   toSend.Print();

   std::string msgPayload = toSend.ToByteStream();

   if( msgPayload.length() < TextProtocol::Message::BASE_PACKET_SIZE )
      throw std::logic_error( "no point in sending an incomplete message" );

   //if( msgPayload.length() != toSend.Size() )
   //   throw std::logic_error( "how did I fuck that up =?" );

   std::cout << "Socket::Send >> " << GetServerAddr() << ":" << GetServerPort() << std::endl;
   const auto bytesSent = CActiveSocket::Send( reinterpret_cast<const uint8*>( &msgPayload.front() ),
                                               msgPayload.length() );

   return ( static_cast<size_t>(bytesSent) == msgPayload.length() );
}

TextProtocol::Message TextProtocol::Socket::Receive()
{
      std::cout << "Socket::Receive >> waiting for message from "
                << GetClientAddr() << ":" << GetClientPort() <<std::endl;

   auto bytesObtained = -1;
   if( ( bytesObtained = CActiveSocket::Receive( Message::MAX_MESSAGE_SIZE ) ) > 0)
   {
      std::cout << "Socket::Receive >> " << bytesObtained << " from "
                << GetClientAddr() << ":" << GetClientPort() <<std::endl;
      std::string bytesRx{ reinterpret_cast<const char*>( GetData() ),
                           static_cast<size_t>( bytesObtained ) };

      return Message::Parse( bytesRx );
   }
   else
   {
      std::cout << "Socket::Receive >> " << DescribeError() <<std::endl;
   }

   return{ TextProtocol::PacketType::NACK, 
           TextProtocol::SequenceNumber{},
           TextProtocol::IpV4Address{},
           TextProtocol::PortNumber{}
         };
}

bool TextProtocol::Socket::Close()
{
   return CActiveSocket::Close();
}
