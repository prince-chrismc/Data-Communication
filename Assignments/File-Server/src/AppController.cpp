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
#include <iostream>
#include <thread>
#include "IconServlet.h"
#include "FileServlet.h"

using namespace std::chrono_literals;

AppController::AppController( int argc, char ** argv ) : m_CliParser( argc, argv ), m_Verbose( false ), m_Port( 8080 ), m_FileExplorerRoot( "." )
{
}

void AppController::Initialize()
{
   m_Verbose = m_CliParser.DoesSwitchExists( "-v" );

   if( m_CliParser.DoesSwitchExists( "-p" ) )
   {
      try
      {
         m_Port = static_cast<unsigned short>( std::stoul( *++m_CliParser.find( "-p" ) ) );
      }
      catch( ... )
      {
         printGeneralUsage();
         throw std::logic_error( "Invalid port number specified!" );
      }
   }

   if( m_CliParser.DoesSwitchExists( "-d" ) )
   {
      try
      {
         m_FileExplorerRoot = *++m_CliParser.find( "-d" );
      }
      catch( ... )
      {
         printGeneralUsage();
         throw std::logic_error( "Incorrectly specified directory!" );
      }
   }

   if( m_CliParser.DoesSwitchExists( "-i" ) )
   {
      try
      {
         m_FaviconPath = *++m_CliParser.find( "-i" );
      }
      catch( ... )
      {
         printGeneralUsage();
         throw std::logic_error( "Incorrectly favicon location!" );
      }
   }
}

void AppController::Run()
{
   HttpServer oServer;
   std::unique_ptr<FileServlet> oFileExplorer = std::make_unique<FileServlet>( m_FileExplorerRoot );
   oServer.RegisterServlet( "/", oFileExplorer.get() );

   std::unique_ptr<IconServlet> oFavicon;
   if( m_FaviconPath.length() )
   {
      oFavicon = std::make_unique<IconServlet>( m_FaviconPath );
      oServer.RegisterServlet( "/favicon.ico", oFavicon.get() );
   }

   if( m_Verbose )
      std::cout << "Successfully created sevlets" <<std::endl;

   oServer.Launch( m_Port );

   if( m_Verbose )
      std::cout << "Successfully launch http server now ready to answer!" <<std::endl;

   std::this_thread::sleep_for( 1h );

   oServer.Close();
}

//
// Printing
//
void AppController::printGeneralUsage()
{
   /*
    * General Usage
    *    httpfs help
    * httpfs is a simple HTTP based file server.
    * usage:
    *    httpfs [-v] [-p PORT] [-d PATH-TO-DIR] [-i ICON-PATH]
    * -v Prints debugging messages.
    * -p Specifies the port number that the server will listen and serve at. Default is 8080.
    * -d Specifies the directory that the server will use to read/write requested files. Default is the current directory when launching the application.
    * -i Specifies the path to the favorite icon saved in a PNG format.
    */

   std::cout << "General Usage\r\n   httpfs help\r\nhttpfs is a simple file server.\r\nUsage:\r\n   hhttpfs [-v] [-p PORT] [-d PATH-TO-DIR] [-i ICON-PATH]\r\n";
   std::cout << "-v   Prints debugging messages.\r\n-p Specifies the port number that the server will listen and serve at. Default is 8080.\r\n";
   std::cout << "-d Specifies the directory that the server will use to read/write requested files. Default is the current directory when launching the application.\r\n";
   std::cout << "-i Specifies the path to the favorite icon saved in a PNG format." << std::endl;
}
