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

#include "AppController.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Socket.h"

#ifdef _WIN32
#include <Ws2tcpip.h>
#endif

#ifdef LINUX_PLATFORM
#include <arpa/inet.h>
#endif

CurlAppController::CurlAppController( int argc, char ** argv )
   : m_oCliParser( argc, argv )
   , m_eCommand( Http::RequestMethod::Invalid )
   , m_bVerbose( false )
   , m_Client( CSimpleSocket::SocketTypeUdp )
{
   readCommandLineArgs();
}

CurlAppController::~CurlAppController()
{
   debugPrint( "Closing...", "\r\n" );
   m_Client.Close();
}

void CurlAppController::readCommandLineArgs()
{
   auto itor = m_oCliParser.cbegin();
   moreArgsToRead( itor, MISSING_GET_OR_POST );

   enum Options { HELP, GET, POST };
   switch( CommandLineParser::doesMatch<Options>( itor, { "help", "get", "post" } ) )
   {
   case HELP:
      printUsageGivenArgs();
      break;
   case GET:
      m_eCommand = Http::RequestMethod::Get;
      parseGetOptions( ++itor );
      break;
   case POST:
      m_eCommand = Http::RequestMethod::Post;
      parsePostOptions( ++itor );
      break;

   default:
      printUsageGivenArgs();
      throw std::invalid_argument( MISSING_GET_OR_POST.data() );
   }
}

void CurlAppController::Run()
{
   validateCommand();

   establishConnection();

   sendHttpRequest();

   receiveHttpResponse();

   std::cout.flush();
}

//
// Printing
//
void CurlAppController::printGeneralUsage()
{
   std::cout << "General Usage\r\n   httpc help\r\nhttpc is a curl - like application but supports HTTP protocol only.\r\nUsage:\r\n   httpc command [ arguments ]\r\nThe commands are:\r\n";
   std::cout << "   get     executes a HTTP GET request and prints the response.\r\n   post    executes a HTTP POST request and prints the response.\r\n";
   std::cout << "Other arguments are:\r\n   help    prints extremely helpful screen.\r\nUse 'httpc help [ command ]' for more information about a command." << std::endl;
}

void CurlAppController::printGetUsage()
{
   std::cout << "Get Usage\r\n   httpc help get\r\nGet executes a HTTP GET request for a given URL.\r\nUsage:\r\n   httpc get [ -v ] [ -h key:value ] URL\r\n";
   std::cout << "-v             Prints the detail of the response such as protocol, status, and headers.\r\n";
   std::cout << "-h key:value   Associates headers to the HTTP Request with the format 'key:value'. Can specify many in a row.\r\n" << std::endl;
}

void CurlAppController::printPostUsage()
{
   std::cout << "Post Usage\r\nhttpc help post\r\nPost executes a HTTP POST request for a given URL with inline data or from file.\r\nUsage:\r\n";
   std::cout << "   httpc post [ -v ] [ -h key:value ] [ -d inline-data | -f file ] URL\r\n";
   std::cout << "-v             Prints the detail of the response such as protocol, status,and headers.\r\n";
   std::cout << "-h key:value   Associates headers to HTTP Request with the format 'key:value'. Can specify many in a row.\r\n";
   std::cout << "-d string      Associates an inline data to the body HTTP POST request.\r\n";
   std::cout << "-f file        Associates the content of a file to the body HTTP POST request.\r\n";
   std::cout << "Either [ -d ] or [ -f ] can be used but not both." << std::endl;
}

void CurlAppController::printUsageGivenArgs() const
{
   switch( m_eCommand )
   {
   case Http::RequestMethod::Get: printGetUsage(); break;
   case Http::RequestMethod::Post: printPostUsage(); break;
   default: printGeneralUsage(); break;
   }
}

//
// Prasing
//
void CurlAppController::parseGetOptions( CommandLineParser::ArgIterator itor )
{
   parseVerboseOption( itor );
   parseHeaderOption( itor );
   parseUrlOption( itor );
}

void CurlAppController::parsePostOptions( CommandLineParser::ArgIterator itor )
{
   parseVerboseOption( itor );
   parseHeaderOption( itor );

   moreArgsToRead( itor, MISSING_URL );

   if( *itor == "-d" )
   {
      moreArgsToRead( ++itor, MISSING_URL );

      m_sBody = *itor;
      ++itor;
   }
   else if( *itor == "-f" )
   {
      moreArgsToRead( ++itor, MISSING_URL );

      std::ifstream fileReader( *itor, std::ios::in | std::ios::binary | std::ios::ate );
      if( !fileReader ) { printPostUsage(); throw std::invalid_argument( "Unable to use file specified with -f switch" ); }

      const size_t size = fileReader.tellg();
      m_sBody.resize( size + 1, '\0' ); // construct buffer
      fileReader.seekg( 0 ); // rewind
      fileReader.read( m_sBody.data(), size );

      ++itor;
   }

   parseUrlOption( itor );
}

void CurlAppController::parseVerboseOption( CommandLineParser::ArgIterator& itor )
{
   moreArgsToRead( itor, MISSING_URL );

   if( *itor == "-v" )
   {
      m_bVerbose = true;
      ++itor;
   }
}

void CurlAppController::parseHeaderOption( CommandLineParser::ArgIterator& itor )
{
   moreArgsToRead( itor, MISSING_URL );

   while( *itor == "-h" )
   {
      std::string sHeardOptAndValue( *( ++itor ) );
      const size_t iSeperatorIndex = sHeardOptAndValue.find( ':' );
      if( iSeperatorIndex == std::string::npos ) { printUsageGivenArgs(); throw std::invalid_argument( "Poorly formatted key:value for -h switch" ); }

      m_oExtraHeaders.emplace_back( sHeardOptAndValue.substr( 0, iSeperatorIndex ), sHeardOptAndValue.substr( iSeperatorIndex + 1 ) );
      ++itor;

      moreArgsToRead( itor, MISSING_URL );
   }
}

void CurlAppController::parseUrlOption( CommandLineParser::ArgIterator & itor )
{
   moreArgsToRead( itor, MISSING_URL );

   try
   {
      m_oHref = HrefParser().Parse( *itor ).GetHref();
   }
   catch( const HrefParser::ParseError& e )
   {
      printUsageGivenArgs();
      throw e;
   }
}

void CurlAppController::moreArgsToRead( CommandLineParser::ArgIterator itor, std::string_view errMsg ) const
{
   if( itor == m_oCliParser.cend() )
   {
      printUsageGivenArgs();
      throw std::invalid_argument( errMsg.data() );
   }
}

void CurlAppController::validateCommand() const
{
   switch( m_eCommand )
   {
   case Http::RequestMethod::Get:
   case Http::RequestMethod::Post:
      break;
   default:
      throw std::runtime_error( "If you see this please don't look for the developer to report a bug =)" );
   }
}

void CurlAppController::establishConnection()
{
   debugPrint( "Connectioning to router...\r\n" );

   if( !m_Client.Open( m_oHref.m_sHostName.c_str(), 3000 ) )
   {
      debugPrint( "Establish connection failed because ", m_Client.DescribeError(), "\r\n" );
      throw std::runtime_error( "Failed to establish connection with router" );
   }



   sockaddr_in sa{};
   // store this IP address in sa:
   inet_pton( AF_INET, m_Client.GetServerAddr().c_str(), &( sa.sin_addr ) );
   const TextProtocol::IpV4Address serverIp{ sa.sin_addr.s_addr };

   const TextProtocol::PortNumber serverPort{ 8080 };

   const TextProtocol::Message synMessage( TextProtocol::PacketType::SYN, m_Expected++, serverIp, serverPort );

   debugPrint( "Attempting to connect with Server... Sending >> ", synMessage, "\r\n" );
   if( !TextProtocol::Socket::Send( m_Client, synMessage ) )
   {
      debugPrint( "Unable to send message because ", m_Client.DescribeError(), "\r\n" );
      throw std::runtime_error( "Failed to send SYN to server" );
   }

   debugPrint( "Waiting for SYN_ACK...", "\r\n" );

   auto synackMessage = TextProtocol::Socket::Receive( m_Client );

   if( !synackMessage.has_value() )
   {
      debugPrint( "Received failed due to: ", m_Client.DescribeError(), "\r\n" );
      throw std::runtime_error( "Failed to receive SYN_ACK from server" );
   }

   if( synackMessage->m_PacketType != TextProtocol::PacketType::SYN_ACK ||
       synackMessage->m_SeqNum != m_Expected )
   {
      debugPrint( "Obtained >> ", *synackMessage, "\r\n" );
      throw std::runtime_error( "SYN_ACK did not match expected seq num" );
   }

   const TextProtocol::Message ackMessage( TextProtocol::PacketType::SYN_ACK, m_Expected++, serverIp, serverPort );

   debugPrint( "Completing three-way hand shake... Sending >> ", ackMessage, "\r\n" );
   if( !TextProtocol::Socket::Send( m_Client, ackMessage ) )
   {
      debugPrint( "Unable to send message because ", m_Client.DescribeError(), "\r\n" );

      // Maybe throw -- depends on sever implementation
   }

   if( m_bVerbose ) std::cout << "Successfully connected to server!" << std::endl;
}

void CurlAppController::sendHttpRequest()
{
   debugPrint( "Building Request..." );

   HttpRequest oReq( m_eCommand, m_oHref.m_sUri, Http::Version::v10, m_oHref.m_sHostName + ":" + std::to_string( m_oHref.m_nPortNumber ) );
   for( auto& oFeildNameAndValue : m_oExtraHeaders )
   {
      oReq.SetMessageHeader( oFeildNameAndValue.first, oFeildNameAndValue.second );
   }
   oReq.AppendMessageBody( m_sBody );

   sockaddr_in sa{};
   // store this IP address in sa:
   inet_pton( AF_INET, m_Client.GetServerAddr().c_str(), &( sa.sin_addr ) );
   const TextProtocol::IpV4Address serverIp{ sa.sin_addr.s_addr };
   const TextProtocol::PortNumber serverPort{ 8080 };

   TextProtocol::Message request( TextProtocol::PacketType::ACK, m_Expected++, serverIp, serverPort );
   request.m_Payload = oReq.GetWireFormat();

   debugPrint( " Sending >> ", request, " for ", oReq.GetRequestLine(), "\r\n" );
   if( !TextProtocol::Socket::Send( m_Client, request ) )
      throw std::runtime_error( "Failed to send HTTP request" );
}

void CurlAppController::receiveHttpResponse()
{
   debugPrint( "Receiving... " );
   auto response = TextProtocol::Socket::Receive( m_Client );
   if( response.has_value() )
   {
      HttpResponseParser oParser;
      oParser.AppendResponseData( response->m_Payload );
      auto httpResponse = oParser.GetHttpResponse();

      debugPrint( *response, httpResponse.GetStatusLine(), "\r\n\r\nHere's the response!" );

      if( m_bVerbose )
      {
         std::cout << httpResponse.GetWireFormat();
      }
      else
      {
         std::cout << httpResponse.GetBody();
      }
   }
   else
   {
      debugPrint( "Received failed due to: ", m_Client.DescribeError() );
   }
}
