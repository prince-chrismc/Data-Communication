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

#include "CurlAppController.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "ActiveSocket.h"

CurlAppController::CurlAppController( int argc, char ** argv )
   : m_oCliParser( argc, argv )
   , m_eCommand( Http::RequestMethod::Invalid )
   , m_bVerbose( false )
{
   readCommandLineArgs();
}

void CurlAppController::readCommandLineArgs()
{
   auto itor = m_oCliParser.cbegin();
   moreArgsToRead( itor, MISSING_GET_OR_POST );

   if( *itor == "help" ) printUsageGivenArgs();
   else if( *itor == "get" ) m_eCommand = Http::RequestMethod::Get;
   else if( *itor == "post" ) m_eCommand = Http::RequestMethod::Post;
   else { printUsageGivenArgs(); throw std::invalid_argument( MISSING_GET_OR_POST.data() ); }

   switch( m_eCommand )
   {
   case Http::RequestMethod::Get:
      // Continue parsing GET args
      parseGetOptions( ++itor );
      break;
   case Http::RequestMethod::Post:
      // Continue parsing POST args
      parsePostOptions( ++itor );
      break;
   default:
      break;
   }
}

void CurlAppController::Run()
{
   switch( m_eCommand )
   {
   case Http::RequestMethod::Get:
   case Http::RequestMethod::Post:
      break;
   default:
      throw std::runtime_error( "If you see this please don't look for the developer to report a bug =)" );
   }

   if( m_bVerbose ) std::cout << "Starting..." << std::endl;
   CActiveSocket oClient;

   bool retval = true;

   if( retval )
   {
      if( m_bVerbose ) std::cout << "Connectioning to " << m_oHref.m_sHostName << " on port " << 80 << "..." << std::endl;
      retval = oClient.Open( m_oHref.m_sHostName.c_str(), m_oHref.m_nPortNumber );

      if( !retval && m_bVerbose ) std::cout << "Connection could not be established!" << std::endl;
   }

   if( retval )
   {
      if( m_bVerbose ) std::cout << "Building Request..." << std::endl;
      HttpRequest oReq( m_eCommand, m_oHref.m_sUri, Http::Version::v10, m_oHref.m_sHostName + std::to_string( m_oHref.m_nPortNumber ) );
      for( auto& oFeildNameAndValue : m_oExtraHeaders )
      {
         oReq.SetMessageHeader( oFeildNameAndValue.first, oFeildNameAndValue.second );
      }
      oReq.AppendMessageBody( m_sBody );
      std::string sRawRequest = oReq.GetWireFormat();

      if( m_bVerbose ) std::cout << "Raw request:" << std::endl << std::endl << sRawRequest << std::endl << std::endl << "Sending...";
      retval = oClient.Send( (uint8_t*)sRawRequest.c_str(), sRawRequest.size() );
   }

   HttpResponseParser oResponseParserParser;
   if( retval )
   {
      if( m_bVerbose ) std::cout << "Receiving..." << std::endl;
      int32_t bytes_rcvd = -1;
      do
      {
         bytes_rcvd = oClient.Receive( 1024 );

         if( bytes_rcvd <= 0 ) break;

         if( m_bVerbose ) std::cout << "Appending " << bytes_rcvd << " bytes of data..." << std::endl;

      } while( !oResponseParserParser.AppendResponseData( oClient.GetData() ) );

      if( m_bVerbose ) std::cout << "Transmission Completed..." << std::endl;
   }

   if( retval )
   {
      HttpResponse oRes = oResponseParserParser.GetHttpResponse();

      if( m_bVerbose ) std::cout << "Closing..." << std::endl;
      oClient.Close();
      if( m_bVerbose ) std::cout << std::endl << std::endl << "Here's the response!" << std::endl << std::endl;

      std::cout << oRes.GetWireFormat();
      std::cout.flush();
   }
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
