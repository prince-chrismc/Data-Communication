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

#include <stdexcept>
#include <iostream>
#include "Socket.h"

bool TextProtocol::Socket::Send( CSimpleSocket& socket, const Message& toSend )
{
   //std::cout << "Socket::Send >> " << toSend << std::endl;

   std::string msgPayload = toSend.ToByteStream();

   if( msgPayload.length() < Message::BASE_PACKET_SIZE )
      throw std::logic_error( "no point in sending an incomplete message" );

   if( msgPayload.length() != toSend.Size() )
      throw std::logic_error( "how did I fuck that up =?" );

   //std::cout << "Socket::Send >> " << socket.GetServerAddr() << ":" << socket.GetServerPort() << std::endl;
   const auto bytesSent = socket.Send( reinterpret_cast<const uint8*>( &msgPayload.front() ), msgPayload.length() );

   return ( static_cast<size_t>( bytesSent ) == toSend.Size() );
}

std::optional<TextProtocol::Message> TextProtocol::Socket::Receive( CSimpleSocket& socket )
{
   auto bytesObtained = -1;
   if( ( bytesObtained = socket.Receive( Message::MAX_MESSAGE_SIZE ) ) > 0 )
   {
      //std::cout << "Socket::Receive >> " << bytesObtained << " from "
      //   << socket.GetClientAddr() << ":" << socket.GetClientPort() << std::endl;

      return Message::Parse( socket.GetData() );
   }

   //std::cout << "Socket::Receive >> " << socket.DescribeError() << std::endl;
   return{};
}
