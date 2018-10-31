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

#include <vector>
#include <future>
#include <memory>
#include <map>
#include <mutex>
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "Constants.h"
#include "PassiveSocket.h"

class HttpServlet
{
public:
   virtual ~HttpServlet() = default;
   virtual HttpResponse HandleRequest( const HttpRequest& request ) const noexcept = 0;
};

//
// A RESTful API style HTTP server designed for simplicity with moderate performance.
class HttpServer
{
public:
   HttpServer( HttpVersion version = HttpVersion11 );
   ~HttpServer();

   bool RegisterServlet( const char* uri, HttpServlet* servlet );

   bool Launch( const char* addr, int nPort );

   bool Close();

private:
   const HttpVersion m_eVersion;
   CPassiveSocket m_oSocket;

   std::unique_ptr<std::promise<void>> m_pExitEvent;


   std::mutex m_muConnectionList;
   std::vector<std::pair<std::chrono::steady_clock::time_point, std::shared_ptr<CActiveSocket>>> m_vecClients;

   std::condition_variable m_cvCleanSignal;


   struct UriComparator
   {
      //
      // Conditions for sorting URIs
      // - Number of slashes
      // - Left of relative path
      bool operator()( const std::string& lhs, const std::string& rhs ) const;
   };

   std::map<std::string, HttpServlet*, UriComparator> m_RestfulServlets;


   HttpServlet* BestMatchingServlet( const std::string& uri ) const;
   void NonPersistentConnection( std::shared_ptr<CActiveSocket> pClient ) const noexcept;
};
