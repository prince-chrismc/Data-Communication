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

#pragma once

#include <string>

// In little endian, a 32bit integer value of 1 is represented in hex as `0x01 0x00 0x00 0x00`
constexpr auto TEST_BYTE = 1;
constexpr bool IS_LITTLE_ENDIAN = ( TEST_BYTE == 0x10000000 );
constexpr bool IS_BIG_ENDIAN = ( TEST_BYTE == 0x00000001 );

namespace TextProtocol
{
   template <typename Enum>
   auto operator++( Enum& e, int ) noexcept
   {
      static_assert( std::is_enum<Enum>::value, "type must be an enum" );

      Enum temp = e;
      auto newValue = static_cast<std::underlying_type_t<Enum>>( e );
      ++newValue;
      e = Enum{ newValue };
      return temp;
   }

   template <typename Enum>
   Enum& operator++( Enum& e ) noexcept
   {
      static_assert( std::is_enum<Enum>::value, "type must be an enum" );

      auto newValue = static_cast<std::underlying_type_t<Enum>>( e );
      ++newValue;
      e = Enum{ newValue };
      return e;
   }

   enum class PacketType : unsigned char
   {
      ACK = 0x06,
      NACK = 0x15,
      SYN = 0x16,
      SYN_ACK = SYN + ACK
   };

   enum class SequenceNumber : unsigned long { MAX = 0xffffffffUL };

   enum class IpV4Address : unsigned long { };

   enum class PortNumber : unsigned short { };


   class Message
   {
   public:
      Message( PacketType type, SequenceNumber id, IpV4Address dstIp, PortNumber port );

      size_t Size() const;
      std::string ToByteStream() const;

      friend std::ostream& operator<<(std::ostream& os, const Message& message );

      static Message Parse( const std::string& rawBytes );

      static constexpr auto BASE_PACKET_SIZE = sizeof( PacketType ) + sizeof( SequenceNumber ) + sizeof( IpV4Address ) + sizeof( PortNumber );
      static constexpr auto MAX_PAYLOAD_LENGTH = 1013;
      static constexpr auto MAX_MESSAGE_SIZE = BASE_PACKET_SIZE + MAX_PAYLOAD_LENGTH;

      PacketType m_PacketType;
      SequenceNumber m_SeqNum;
      IpV4Address m_DstIp;
      PortNumber m_DstPort;
      std::string m_Payload = "Hello World!"; // max 1014 bytes
   };
}
