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

#include "HttpServer.h"
#include <iterator>
#include <sstream>
#include <algorithm>
#include <iostream>

using namespace std::chrono_literals;

HttpServer::HttpServer( Http::Version version /*= v11*/ )
   : m_eVersion( version )
   , m_pExitEvent( std::make_unique<std::promise<void>>() )
{
}

bool HttpServer::RegisterServlet( const char * uri, HttpServlet * servlet )
{
   if( uri == nullptr || uri[ 0 ] != '/' ) return false;

   return m_RestfulServlets.try_emplace( uri, servlet ).second;
}

void HttpServer::Launch( unsigned short port )
{
   if( !m_oSocket.Listen( nullptr, port ) )
      throw std::runtime_error( "Unable to bind HTTP Server" );

   auto oExitEvent = std::make_shared<std::shared_future<void>>( m_pExitEvent->get_future() );

   std::thread( [ this, oExitEvent ]
                {
                   while( oExitEvent->wait_for( 10ms ) == std::future_status::timeout )
                   {
                      std::unique_lock<std::mutex> cleanLock( m_muConnectionList );
                      if( m_cvCleanSignal.wait_for( cleanLock, 20ms, [ this ] { return !m_vecClients.empty(); } ) )
                      {
                         for( auto itor = m_vecClients.begin(); itor != m_vecClients.end(); /* no itor */ )
                         {
                            if( !ConnectionIsAlive( itor->get() ) )
                            {
                               if( ( *itor )->m_pClient != nullptr ) ( *itor )->m_pClient->Shutdown( CSimpleSocket::Both );
                               itor = m_vecClients.erase( itor );
                            }
                            else
                            {
                               ++itor;
                            }
                         }

                         m_vecClients.shrink_to_fit();
                      }
                   }
                }
   ).detach();

   std::thread( [ this, oExitEvent ]
                {
                   std::function<void( std::shared_ptr<ClientConnection> )> HandleNewConnection;

                   switch( m_eVersion )
                   {
                   case Http::Version::v10:
                      HandleNewConnection = [ this ]( std::shared_ptr<ClientConnection> pClient ) { NonPersistentConnection( pClient.get() ); };
                      break;
                   case Http::Version::v11:
                      HandleNewConnection = [ this ]( std::shared_ptr<ClientConnection> pClient ) { PersistentConnection( pClient.get() ); };
                      break;
                   default:
                      throw std::invalid_argument( "Bad HTTP version!" );
                   }

                   while( oExitEvent->wait_for( 10ms ) == std::future_status::timeout )
                   {
                      std::shared_ptr<CActiveSocket> pClient;
                      if( ( pClient = m_oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
                      {
                         std::cout << "New client obtained { " << std::hex << pClient.get() << " }" << std::endl;

                         std::lock_guard<std::mutex> oAutoLock( m_muConnectionList );
                         m_vecClients.push_back( std::make_shared<ClientConnection>( std::move( pClient ) ) );

                         std::thread( HandleNewConnection, m_vecClients.back() ).detach();
                      }
                   }
                }
   ).detach();
}

bool HttpServer::Close()
{
   bool bRetVal = m_oSocket.Shutdown( CSimpleSocket::Both );

   if( bRetVal )
   {
      bRetVal = m_oSocket.Close();
   }

   m_pExitEvent->set_value();

   return bRetVal;
}

HttpServer::ClientConnection::ClientConnection( std::shared_ptr<CActiveSocket>&& client ) : m_pClient( client )
{
}

HttpServlet* HttpServer::BestMatchingServlet( const std::string & uri ) const
{
   for( auto itor = m_RestfulServlets.crbegin(); itor != m_RestfulServlets.crend(); ++itor )
   {
      if( uri.compare( 0, itor->first.length(), itor->first ) == 0 ) return itor->second;
   }

   return nullptr;
}

bool HttpServer::ConnectionIsAlive( ClientConnection* pConnection )
{
   return std::chrono::steady_clock::now() - pConnection->m_tLastSighting <= 100s &&
      pConnection->m_pClient->IsSocketValid() &&
      pConnection->m_nRemainingRequests > 0;

}

void HttpServer::NonPersistentConnection( ClientConnection* pConnection ) const
{
   auto pClient = pConnection->m_pClient.get();
   auto oPotentialRequest = ReadNextRequest( pClient );

   if( oPotentialRequest.has_value() )
   {
      ProcessNewRequest( pConnection, oPotentialRequest.value() );
   }

   pClient->Close();
}

void HttpServer::PersistentConnection( ClientConnection* pConnection ) const
{
   auto pClient = pConnection->m_pClient.get();

   do
   {
      auto oPotentialRequest = ReadNextRequest( pClient );

      if( oPotentialRequest.has_value() )
         ProcessNewRequest( pConnection, oPotentialRequest.value() );

   } while( ConnectionIsAlive( pConnection ) );

   pClient->Close();
}

std::optional<HttpRequest> HttpServer::ReadNextRequest( CActiveSocket* pClient )
{
   HttpRequestParser oParser;
   int32_t bytes_rcvd = -1;
   do
   {
      bytes_rcvd += pClient->Receive( 2048 );

      if( bytes_rcvd <= 0 )
      {
         pClient->Close();
         return{};
      }

   } while( !oParser.AppendRequestData( pClient->GetData() ) );

   return oParser.GetHttpRequest();
}

void HttpServer::ProcessNewRequest( ClientConnection* pConnection, const HttpRequest& oRequest ) const
{
   pConnection->m_tLastSighting = std::chrono::steady_clock::now();
   std::cout << "New request from { " << std::hex << pConnection->m_pClient.get() << " }. Remaining :" << std::dec << pConnection->m_nRemainingRequests << std::endl;

   HttpResponse oResponse = BestMatchingServlet( oRequest.GetUri() )->HandleRequest( oRequest );
   //std::cout << "{ " << std::hex << pConnection->m_pClient.get() << " } " << oRequest.GetRequestLine() << " --> " << oResponse.GetStatusLine() << std::endl;

   // TODO : Handle HTTP Headers
   oResponse.SetMessageHeader( "Server", "HTTP Server by Christopher McArthur" );

   if( oRequest.GetVersion() == Http::Version::v11 &&
       oResponse.GetVersion() == Http::Version::v11 && 
       pConnection->m_nRemainingRequests > 1 )
      oResponse.SetMessageHeader( "Keep-Alive", "timeout=100, max=" + std::to_string( pConnection->m_nRemainingRequests ) );
   else
      oResponse.SetMessageHeader( "Connection", "closed" );

   std::string sRawRequest = oResponse.GetWireFormat();
   pConnection->m_pClient->Send( reinterpret_cast<const uint8_t*>( sRawRequest.c_str() ), sRawRequest.size() );
   pConnection->m_nRemainingRequests -= 1;

   if( oRequest.GetVersion() != Http::Version::v11 ||
       oResponse.GetVersion() != Http::Version::v11 ||
       !ConnectionIsAlive( pConnection ) )
   {
      pConnection->m_pClient->Close();
   }
}

bool HttpServer::UriComparator::operator()( const std::string & lhs, const std::string & rhs ) const
{
   if( lhs == "/" )
   {
      return true;
   }
   if( rhs == "/" )
   {
      return false;
   }
   if( std::count( lhs.begin(), lhs.end(), '/' ) < std::count( rhs.begin(), rhs.end(), '/' ) )
   {
      return true;
   }
   else if( std::count( lhs.begin(), lhs.end(), '/' ) == std::count( rhs.begin(), rhs.end(), '/' ) )
   {
      std::vector<std::string> tokens_lhs = tokenizeUri( lhs );
      std::vector<std::string> tokens_rhs = tokenizeUri( rhs );

      for( size_t i = 0; i < tokens_rhs.size(); i++ )
      {
         if( tokens_lhs.at( i ) != tokens_rhs.at( i ) )
         {
            return tokens_lhs.at( i ) < tokens_rhs.at( i );
         }
      }
   }

   return false;
}

std::vector<std::string> HttpServer::UriComparator::tokenizeUri( const std::string& uri )
{
   std::istringstream iss_rhs( reduce( uri, " ", "/" ) );
   return{ std::istream_iterator<std::string>{iss_rhs}, std::istream_iterator<std::string>{} };
}
