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

using namespace std::chrono_literals;

HttpServer::HttpServer( HttpVersion version /*= HttpVersion11*/ ) : m_eVersion( version )
{
}

HttpServer::~HttpServer()
{
   //m_oExitEvent.set_exception( std::make_exception_ptr( std::runtime_error( "Failed to close server!" ) ) );
}

bool HttpServer::RegisterServlet( const char * uri, HttpServlet * servlet )
{
   return m_RestfulServlets.try_emplace( uri, servlet ).second;
}

bool HttpServer::Launch( const char* addr, int32 nPort )
{
   bool bRetVal = m_oSocket.Initialize();

   if( bRetVal )
   {
      bRetVal = m_oSocket.Listen( addr, nPort );
   }

   if( bRetVal )
   {
      //const auto PersistentConnection = [ oExitEvent = m_oExitEvent.get_future().share(), Servlets = m_RestfulServlets ]( CActiveSocket* pClient )
      //{

      //};


      std::thread( [ this ]
                   {
                      auto oExitEvent = m_oExitEvent.get_future().share();
                      while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
                      {
                         std::unique_ptr<CActiveSocket> pClient;
                         if( ( pClient = m_oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
                         {
                            m_vecClients.push_back( std::move( pClient ) );

                            switch( m_eVersion )
                            {
                            case HttpVersion10:
                            case HttpVersion11:
                               std::thread( [ this ]( CActiveSocket* pClient ) { NonPersistentConnection( pClient ); }, m_vecClients.back().get() ).detach();
                               break;
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

   return bRetVal;
}

bool HttpServer::Close()
{
   bool bRetVal = m_oSocket.Shutdown( CSimpleSocket::Both );

   if( bRetVal )
   {
      bRetVal = m_oSocket.Close();
   }

   m_oExitEvent.set_value();

   return bRetVal;
}

HttpServlet* HttpServer::BestMatchingServlet( const std::string & uri ) const
{
   for( auto itor = m_RestfulServlets.crbegin(); itor != m_RestfulServlets.crend(); itor++ )
   {
      if( itor->first.size() <= uri.size() && itor->first.compare( 0, uri.length(), uri ) == 0 ) return itor->second;
   }

   return nullptr;
}

void HttpServer::NonPersistentConnection( CActiveSocket * pClient ) const noexcept
{
   {
      HttpRequestParserAdvance oParser;
      int32 bytes_rcvd = -1;
      do
      {
         bytes_rcvd = pClient->Receive( 1024 );

         if( bytes_rcvd <= 0 ) break;

      } while( !oParser.AppendRequestData(
         std::string( reinterpret_cast<const char*>( pClient->GetData() ), bytes_rcvd ) ) );

      HttpRequest oRequest = oParser.GetHttpRequest();

      BestMatchingServlet( oRequest.GetUri() );

      //HttpResponse oResponse = 


   }
}
