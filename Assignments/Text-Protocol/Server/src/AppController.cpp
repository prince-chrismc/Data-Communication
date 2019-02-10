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
#include "Message.h"
#include "Socket.h"
#include "HttpRequest.h"
#include "FileServlet.h"
#include <iostream>
#include <thread>
#include <future>

using namespace std::chrono_literals;
using TextProtocol::PacketType;

AppController::AppController( int argc, char** argv ) :
   m_CliParser( argc, argv ),
   m_Verbose( false ),
   m_Port( 8080 ),
   m_RootDir( "." ),
   m_Socket( CSimpleSocket::SocketTypeUdp )
{
   readCommandLineArgs();
}

void AppController::Run()
{
   if( !m_Socket.Listen( "127.0.0.1", m_Port ) )
      throw std::runtime_error( "Failed to listen on port" );

   std::promise<void> exitSignal;
   auto oExitEvent = exitSignal.get_future();

   std::thread( [ this, exitEvent = std::move( oExitEvent ) ]()
   {
      FileServlet oFileExplorer( m_RootDir );

      while( exitEvent.wait_for( 10ms ) == std::future_status::timeout )
      {
         auto input = TextProtocol::Socket::Receive( m_Socket );

         if( input.has_value() )
         {
            std::cout << "Server >> obtained " << m_Socket.GetBytesReceived() << " containing the following: " << *input
               << " from [ " << m_Socket.GetClientAddr() << ":" << m_Socket.GetClientPort() << " ]" << std::endl;

            if( input->m_PacketType == PacketType::SYN )
            {
               input->m_PacketType = PacketType::SYN_ACK;
            }
            else if( input->m_PacketType == PacketType::SYN_ACK )
            {
               std::cout << "New client connection has been established!" << std::endl;
               continue;
            }
            else if( input->m_PacketType == PacketType::ACK )
            {
               HttpResponse response( Http::Version::v10, Http::Status::BadRequest, "BAD REQUEST" );
               HttpRequestParser parser;
               parser.AppendRequestData( input->m_Payload );

               try
               {
                  auto req = parser.GetHttpRequest();

                  if( req.IsValid() )
                  {
                     response = HttpResponse( Http::Version::v10, Http::Status::InternalServerError, "INTERNAL SERVER ERROR" );
                     response = oFileExplorer.HandleRequest( req );
                  }
               }
               catch( const std::exception& e )
               {
                  std::cout << "Error handling request: " << e.what() << std::endl;
               }

               input->m_Payload = response.GetWireFormat().substr( 0, TextProtocol::Message::MAX_PAYLOAD_LENGTH );
            }

            ++input->m_SeqNum;
            std::cout << "Server >> Sending... " << *input << std::endl;
            TextProtocol::Socket::Send( m_Socket, *input );
         }
      }

      m_Socket.Close();
   }
   ).detach();

   std::cout << "Press 'enter' to close." << std::endl;
   getchar();

   exitSignal.set_value();
}

/*
General Usage
   httpfs help
httpfs is a simple HTTP based file server.
usage:
   httpfs [-v] [-p PORT] [-d PATH-TO-DIR]
-v Prints debugging messages.
-p Specifies the port number that the server will listen and serve at. Default is 8080.
-d Specifies the directory that the server will use to read/write requested files. Default is the current directory when launching the application.
 */
void AppController::printGeneralUsage()
{
   std::cout << "General Usage\r\n   httpfs help\r\nhttpfs is a simple file server.\r\nUsage:\r\n   hhttpfs [-v] [-p PORT] [-d PATH-TO-DIR]\r\n";
   std::cout << "-v   Prints debugging messages.\r\n-p Specifies the port number that the server will listen and serve at. Default is 8080.\r\n";
   std::cout << "-d Specifies the directory that the server will use to read/write requested files. Default is the current directory when launching the application.\r\n" << std::endl;
}

void AppController::readCommandLineArgs()
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
         m_RootDir = *++m_CliParser.find( "-d" );
      }
      catch( ... )
      {
         printGeneralUsage();
         throw std::logic_error( "Incorrectly specified directory!" );
      }
   }
}
