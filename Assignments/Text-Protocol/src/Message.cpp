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

constexpr auto toBytes( unsigned long bytes ) noexcept
{
   return bytes;
}

template <typename Enum>
constexpr auto endianSwap( Enum num )
{
   if constexpr( sizeof( Enum )== 4 )
   {
      const uint32_t b0 = ( toBytes( num ) & 0x000000ff ) << 24u;
      const uint32_t b1 = ( toBytes( num ) & 0x0000ff00 ) << 8u;
      const uint32_t b2 = ( toBytes( num ) & 0x00ff0000 ) >> 8u;
      const uint32_t b3 = ( toBytes( num ) & 0xff000000 ) >> 24u;

      return Enum{ b0 | b1 | b2 | b3 };
   }
   else if constexpr( sizeof( Enum ) == 2 )
   {
      const uint16_t b1 = ( toBytes( num ) & 0x00ff ) << 8u;
      const uint16_t b2 = ( toBytes( num ) & 0xff00 ) >> 8u;

      return Enum{ static_cast<uint16_t>(b1 | b2) };
   }

   return Enum{ 0 };
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
   std::string rawBuffer( reinterpret_cast<const char*>( &m_PacketType ), sizeof( m_PacketType ) );

   std::string seqBuffer( reinterpret_cast<const char*>( &m_SeqNum ), sizeof( m_SeqNum ) );
   seqBuffer.insert( seqBuffer.begin(), 4 - seqBuffer.length(), '0' );

   std::string ipBuffer( reinterpret_cast<const char*>( &m_DstIp ), sizeof( m_DstIp ) );
   ipBuffer.insert( ipBuffer.begin(), 4 - ipBuffer.length(), '0' );

   std::string portBuffer( reinterpret_cast<const char*>( &m_DstPort ), sizeof( m_DstPort ) );
   portBuffer.insert( portBuffer.begin(), 2 - portBuffer.length(), '0' );

   rawBuffer += seqBuffer + ipBuffer + portBuffer;

   return rawBuffer + m_Payload;
}

void TextProtocol::Message::Print() const
{
   std::cout << "Message: " << ToByteStream() << std::endl;
}

TextProtocol::Message TextProtocol::Message::Parse( const std::string & rawBytes )
{
   // TO DO: throw parse error is length < 11
   unsigned short p1 = rawBytes[ 9 ] & 0x00ffu;
   unsigned short p2 = rawBytes[ 10 ] << 8u;

   auto port{ p1 | p2 };

   const uint32_t b0 = rawBytes[ 5 ] << 24u;
   const uint32_t b1 = rawBytes[ 6 ] << 16u;
   const uint32_t b2 = rawBytes[ 7 ] << 8u;
   const uint32_t b3 = rawBytes[ 8 ];

   const uint32_t ipAddr{ b0 | b1 | b2 | b3 };

   return{ PacketType{ static_cast<unsigned char>( rawBytes[ 0 ] ) },
           endianSwap( SequenceNumber{ static_cast<unsigned long>( rawBytes[ 1 ] << 24u | rawBytes[ 2 ] << 16u | rawBytes[ 3 ] << 8u | rawBytes[ 4 ] ) } ),
           endianSwap( IpV4Address{ static_cast<unsigned long>( rawBytes[ 5 ] << 24u | rawBytes[ 6 ] << 16u | rawBytes[ 7 ] << 8u | rawBytes[ 8 ] ) } ),
           endianSwap( PortNumber{ static_cast<unsigned short>( rawBytes[ 9 ] << 8u | ( rawBytes[ 10 ] & 0x00ff ) ) } )
   };
}
