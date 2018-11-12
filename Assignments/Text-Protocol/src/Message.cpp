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

#include "Message.h"
#include <iostream>

template <typename Enum>
constexpr auto toBytes( Enum e ) noexcept // https://stackoverflow.com/a/33083231/8480874
{
   return static_cast<std::underlying_type_t<Enum>>( e );
}

constexpr uint32_t operator&( const TextProtocol::SequenceNumber& lhs, const int& rhs )
{
   return ( static_cast<unsigned long>( lhs ) & static_cast<unsigned long>( rhs ) );
}

constexpr auto endianSwap(TextProtocol::SequenceNumber num )
{
   const uint32_t b0 = (num & 0x000000ff) << 24u;
   const uint32_t b1 = (num & 0x0000ff00) << 8u;
   const uint32_t b2 = (num & 0x00ff0000) >> 8u;
   const uint32_t b3 = (num & 0xff000000) >> 24u;

   return TextProtocol::SequenceNumber{ b0 | b1 | b2 | b3 };
}

TextProtocol::Message::Message( PacketType type, SequenceNumber id, IpV4Address dstIp, PortNumber port ) :
   m_PacketType( type ), m_SeqNum( id ), m_DstIp( dstIp ), m_DstPort( port )
{
   if constexpr( IS_LITTLE_ENDIAN )
   {
      m_SeqNum = endianSwap( m_SeqNum );
   }
}

size_t TextProtocol::Message::Size() const
{
   return BASE_PACKET_SIZE + m_Payload.length();
}

std::string TextProtocol::Message::ToByteStream() const
{
   return std::to_string( toBytes( m_PacketType ) ) + std::to_string( toBytes( m_SeqNum ) )
      + std::to_string( toBytes( m_DstIp ) ) + std::to_string( toBytes( m_DstPort ) ) + m_Payload;
}

void TextProtocol::Message::Print() const
{
   std::cout << "Message: " << ToByteStream() << std::endl;
}

TextProtocol::Message TextProtocol::Message::Parse( const std::string & rawBytes )
{
   return Message( PacketType::ACK, SequenceNumber{ 0 }, IpV4Address{ 0 }, PortNumber{ 0 } );
}
