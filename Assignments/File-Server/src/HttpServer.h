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


#include "HttpResponse.h"
#include "PassiveSocket.h"
#include <vector>
#include <future>
#include <memory>
#include <mutex>
#include <optional>

class HttpServlet
{
public:
   virtual ~HttpServlet() = default;
   virtual HttpResponse HandleRequest( const HttpRequest& request ) const noexcept = 0;
};

//
// A RESTful API style HTTP server designed for simplicity with moderate performance.
//
class HttpServer
{
public:
   HttpServer( Http::Version version = Http::Version::v11 );

   bool RegisterServlet( const char* uri, HttpServlet* servlet );

   void Launch( unsigned short port );

   bool Close();

private:
   const Http::Version m_eVersion;
   CPassiveSocket m_oSocket;

   std::unique_ptr<std::promise<void>> m_pExitEvent;

   std::mutex m_muConnectionList;
   struct ClientConnection
   {
      ClientConnection(std::shared_ptr<CActiveSocket>&& client);

      std::shared_ptr<CActiveSocket> m_pClient;
      std::chrono::steady_clock::time_point m_tLastSighting = std::chrono::steady_clock::now();
      size_t m_nRemainingRequests = 125;
   };
   std::vector<std::shared_ptr<ClientConnection>> m_vecClients;

   std::condition_variable m_cvCleanSignal;

   struct UriComparator
   {
      // Conditions for sorting URIs
      // - Number of slashes
      // - Left of relative path
      bool operator()( const std::string& lhs, const std::string& rhs ) const;

      static std::vector<std::string> tokenizeUri( const std::string& uri );
   };

   std::map<std::string, HttpServlet*, UriComparator> m_RestfulServlets;

   HttpServlet* BestMatchingServlet( const std::string& uri ) const;
   void NonPersistentConnection( ClientConnection* pConnection ) const;
   void PersistentConnection( ClientConnection* pClient ) const;

   static std::optional<HttpRequest> ReadNextRequest( CActiveSocket* pClient );
   void ProcessNewRequest( ClientConnection* pConnection, const HttpRequest& oRequest ) const;

   static bool ConnectionIsAlive( ClientConnection* pConnection );
};
