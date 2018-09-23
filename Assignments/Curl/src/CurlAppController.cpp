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

CurlAppController::CurlAppController( int argc, char ** argv )
   : m_oCliParser( argc, argv )
   , m_eCommand( HttpRequestInvalid )
   , m_bVerbose( false )
{
}

void CurlAppController::Initialize()
{
   auto itor = m_oCliParser.cbegin();

   if( *itor == "get" ) m_eCommand = HttpRequestGet;
   else if( *itor == "post" ) m_eCommand = HttpRequestPost;
   else printGeneralUsage();

   switch( m_eCommand )
   {
   case HttpRequestGet:
      // Continue parsing GET args
      parseGetOptions( ++itor );
      break;
   case HttpRequestPost:
      // Continue parsing POST args
      break;
   default:
      if( *itor != "help" ) throw std::invalid_argument( "Missing GET or POST" );
   }
}

void CurlAppController::Run()
{
}

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

void CurlAppController::printPutUsage()
{
   std::cout << "Post Usage\r\nhttpc help post\r\nPost executes a HTTP POST request for a given URL with inline data or from file.\r\nUsage:\r\n";
   std::cout << "   httpc post [ -v ] [ -h key:value ] [ -d inline-data | -f file ] URL\r\n";
   std::cout << "-v             Prints the detail of the response such as protocol, status,and headers.\r\n";
   std::cout << "-h key:value   Associates headers to HTTP Request with the format 'key:value'. Can specify many in a row.\r\n";
   std::cout << "-d string      Associates an inline data to the body HTTP POST request.\r\n";
   std::cout << "-f file        Associates the content of a file to the body HTTP POST request.\r\n";
   std::cout << "Either [ -d ] or [ -f ] can be used but not both." << std::endl;
}

void CurlAppController::parseGetOptions( CommandLineParser::ArgIterator itor )
{
   if( *itor == "-v" )
   {
      m_bVerbose = true;
      itor++;
   }

   while( *itor == "-h" )
   {
      std::string sHeardOptAndValue( *( ++itor ) );
      const size_t iSeperatorIndex = sHeardOptAndValue.find( ':' );
      if( iSeperatorIndex == std::string::npos ) throw std::invalid_argument( "Poorly formatted key:value for -h switch" );

      m_oExtraHeaders.emplace_back( sHeardOptAndValue.substr( 0, iSeperatorIndex ), sHeardOptAndValue.substr( iSeperatorIndex + 1 ) );
      itor++;
   }

   if( ( *itor ).find( "http://" ) == 0 )
   {
      // TO DO : update with HREF parser
      m_sUrl = *itor;
   }
   else
   {
      printGetUsage();
      throw std::invalid_argument( "Missing URL" );
   }
}
