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

using namespace std::chrono_literals;

HttpServer::HttpServer( HttpVersion version /*= HttpVersion11*/ )
   : m_eVersion( version )
   , m_pExitEvent( std::make_unique<std::promise<void>>() )
{
}

bool HttpServer::RegisterServlet( const char * uri, HttpServlet * servlet )
{
   return m_RestfulServlets.try_emplace( uri, servlet ).second;
}

void HttpServer::Launch( const char* addr, unsigned short nPort )
{
   if( m_oSocket.Listen( addr, nPort ) )
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
                            if( ( std::chrono::steady_clock::now() - itor->first ) > 100s )
                            {
                               if( itor->second != nullptr ) itor->second->Shutdown( CSimpleSocket::Both );
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
                   while( oExitEvent->wait_for( 10ms ) == std::future_status::timeout )
                   {
                      std::shared_ptr<CActiveSocket> pClient;
                      if( ( pClient = m_oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
                      {
                         std::lock_guard<std::mutex> oAutoLock( m_muConnectionList );
                         m_vecClients.emplace_back( std::chrono::steady_clock::now(), pClient );

                         switch( m_eVersion )
                         {
                         case HttpVersion10:
                            std::thread( [ this ]( std::shared_ptr<CActiveSocket> pClient ) { NonPersistentConnection( pClient ); }, m_vecClients.back().second ).detach();
                            break;
                         case HttpVersion11:
                            //std::thread( PersistentConnection, m_vecClients.back().get() ).detach();
                            break;
                         default:
                            throw std::invalid_argument( "Bad HTTP version!" );
                         }
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

HttpServlet* HttpServer::BestMatchingServlet( const std::string & uri ) const
{
   for( auto itor = m_RestfulServlets.crbegin(); itor != m_RestfulServlets.crend(); ++itor )
   {
      if( uri.compare( 0, itor->first.length(), itor->first ) == 0 ) return itor->second;
   }

   return nullptr;
}

void HttpServer::NonPersistentConnection( std::shared_ptr<CActiveSocket> pClient ) const noexcept
{
   HttpRequestParserAdvance oParser;
   int32 bytes_rcvd = -1;
   do
   {
      bytes_rcvd = pClient->Receive( 1024 );

      if( bytes_rcvd <= 0 ) return;

   } while( !oParser.AppendRequestData( pClient->GetData() ) );

   HttpRequest oRequest = oParser.GetHttpRequest();
   HttpResponse oResponse = BestMatchingServlet( oRequest.GetUri() )->HandleRequest( oRequest );

   std::string sRawRequest = oResponse.GetWireFormat();
   pClient->Send( reinterpret_cast<const uint8*>( sRawRequest.c_str() ), sRawRequest.size() );

   pClient->Close();
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
      std::string stripped_lhs = reduce( lhs, " ", "/" );
      std::istringstream iss_lhs( stripped_lhs );
      std::vector<std::string> tokens_lhs{ std::istream_iterator<std::string>{iss_lhs}, std::istream_iterator<std::string>{} };

      std::string stripped_rhs = reduce( rhs, " ", "/" );
      std::istringstream iss_rhs( stripped_rhs );
      std::vector<std::string> tokens_rhs{ std::istream_iterator<std::string>{iss_rhs}, std::istream_iterator<std::string>{} };

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
