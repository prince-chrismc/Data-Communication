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

template <typename Enum, typename Mask>
constexpr auto operator&( const Enum& lhs, const Mask& rhs )->std::underlying_type_t<Enum>
{
   return ( toBytes( lhs ) & rhs );
}

template <typename Enum>
constexpr auto endianSwap( Enum num )
{
   if constexpr( sizeof( Enum ) == 8 ) // hard work done by https://ubuntuforums.org/showthread.php?t=1190710&p=7477357#post7477357
   {
      const uint64_t b0 = ( num & 0x00000000000000ff ) << 56;
      const uint64_t b1 = ( num & 0x000000000000ff00 ) << 40;
      const uint64_t b2 = ( num & 0x0000000000ff0000 ) << 24;
      const uint64_t b3 = ( num & 0x00000000ff000000 ) << 8;
      const uint64_t b4 = ( num & 0x000000ff00000000 ) >> 8;
      const uint64_t b5 = ( num & 0x0000ff0000000000 ) >> 24;
      const uint64_t b6 = ( num & 0x00ff000000000000 ) >> 40;
      const uint64_t b7 = ( num & 0xff00000000000000 ) >> 56;

      return Enum{ b0 | b1 | b2 | b3 | b4 | b5 | b6 | b7 };
   }
   else if constexpr( sizeof( Enum ) == 4 )
   {
      const uint32_t b0 = ( num & 0x000000ff ) << 24u;
      const uint32_t b1 = ( num & 0x0000ff00 ) << 8u;
      const uint32_t b2 = ( num & 0x00ff0000 ) >> 8u;
      const uint32_t b3 = ( num & 0xff000000 ) >> 24u;

      return Enum{ b0 | b1 | b2 | b3 };
   }
   else if constexpr( sizeof( Enum ) == 2 )
   {
      const uint16_t b1 = ( num & 0x00ff ) << 8u;
      const uint16_t b2 = ( num & 0xff00 ) >> 8u;

      return Enum{ static_cast<uint16_t>( b1 | b2 ) };
   }

   return Enum{ num };
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

   auto port = endianSwap( m_DstPort );
   std::string portBuffer( reinterpret_cast<const char*>( &port ), sizeof( port ) );
   portBuffer.insert( portBuffer.begin(), 2 - portBuffer.length(), '0' );

   rawBuffer += seqBuffer + ipBuffer + portBuffer;

   return rawBuffer + m_Payload;
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

   Message obtained{ PacketType{ static_cast<unsigned char>( rawBytes[ 0 ] ) },
           endianSwap( SequenceNumber{ static_cast<unsigned long>( rawBytes[ 1 ] << 24u | rawBytes[ 2 ] << 16u | rawBytes[ 3 ] << 8u | rawBytes[ 4 ] ) } ),
           endianSwap( IpV4Address{ static_cast<unsigned long>( rawBytes[ 5 ] << 24u | rawBytes[ 6 ] << 16u | rawBytes[ 7 ] << 8u | rawBytes[ 8 ] ) } ),
           PortNumber{ static_cast<unsigned short>( rawBytes[ 9 ] << 8u | ( rawBytes[ 10 ] & 0x00ff ) ) }
   };

   obtained.m_Payload = rawBytes.substr( 11 );

   return obtained;
}

std::ostream& TextProtocol::operator<<( std::ostream & os, const TextProtocol::Message & message )
{
   using std::operator<<; // Enable ADL

   operator<<( os, std::string{"MSG: { "});

   os << [ type = message.m_PacketType ]()->std::string
   {
      switch( type )
      {
      case TextProtocol::PacketType::ACK: return "ACK";
      case TextProtocol::PacketType::NACK: return "NACK";
      case TextProtocol::PacketType::SYN: return "SYN";
      case TextProtocol::PacketType::SYN_ACK: return "SYN_ACK";
      default: return "??";
      }
   }( );
   
   operator<<( os, " } Seq=" + std::to_string( toBytes( message.m_SeqNum ) ) + " @=" );

   os << std::to_string( ( message.m_DstIp & 0x000000ff ) >> 0ul ) << std::string{"."}
      << std::to_string( ( message.m_DstIp & 0x0000ff00 ) >> 8ul ) << std::string{"."}
      << std::to_string( ( message.m_DstIp & 0x00ff0000 ) >> 16u ) << std::string{"."}
      << std::to_string( ( message.m_DstIp & 0xff000000 ) >> 24u ) << std::string{":"}
      << std::to_string( toBytes( message.m_DstPort ) );

   return os;
}
