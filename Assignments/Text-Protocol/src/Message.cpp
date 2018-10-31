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

template <typename Enum>
constexpr auto toBytes( Enum e ) noexcept // https://stackoverflow.com/a/33083231/8480874
{
   return static_cast<std::underlying_type_t<Enum>>( e );
}

uint32_t operator&( const TextProtocol::SequenceNumber& lhs, const int& rhs )
{
   return ( static_cast<unsigned long>( lhs ) & static_cast<unsigned long>( rhs ) );
}

TextProtocol::Message::Message( PacketType type, SequenceNumber id, IpV4Address dstIp, PortNumber port ) :
   m_PacketType( type ), m_SeqNum( id ), m_DstIp( dstIp ), m_DstPort( port )
{
   if constexpr( IS_LITTLE_ENDIAN )
   {
      uint32_t b0, b1, b2, b3;

      b0 = ( m_SeqNum & 0x000000ff ) << 24u;
      b1 = ( m_SeqNum & 0x0000ff00 ) << 8u;
      b2 = ( m_SeqNum & 0x00ff0000 ) >> 8u;
      b3 = ( m_SeqNum & 0xff000000 ) >> 24u;

      m_SeqNum = SequenceNumber{ b0 | b1 | b2 | b3 };
   }
}

size_t TextProtocol::Message::Size() const
{
   static constexpr auto BASE_PACKET_SIZE = sizeof( m_PacketType ) + sizeof( m_SeqNum ) + sizeof( m_DstIp ) + sizeof( m_DstPort );
   return BASE_PACKET_SIZE + m_Payload.length();
}

std::string TextProtocol::Message::ToByteStream() const
{
   return std::to_string( toBytes( m_PacketType ) ) + std::to_string( toBytes( m_SeqNum ) )
      + std::to_string( toBytes( m_DstIp ) ) + std::to_string( toBytes( m_DstPort ) ) + m_Payload;
}

TextProtocol::Message TextProtocol::Message::Parse( const std::string & rawBytes )
{
   return Message( PacketType::ACK, SequenceNumber{ 0 }, IpV4Address{ 0 }, PortNumber{ 0 } );
}
