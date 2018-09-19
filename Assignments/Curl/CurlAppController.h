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

#include "CliParser.h"
#include "ActiveSocket.h"

#include "HttpResponse.h"
#include "HttpRequest.h"

#include <stdexcept>

class CurlAppController
{
public:
   CurlAppController( int argc, char** argv ) : m_oCliParser( argc, argv ) {}

   void Initialize()
   {
       auto itor = m_oCliParser.begin();

       if( itor->compare( "GET" ) == 0 ) m_eCommand = HttpRequestGet;
       if( itor->compare( "POST" ) == 0 ) m_eCommand = HttpRequestPost;
       else printGeneralUsage();

       switch( m_eCommand )
       {
           case HttpRequestGet:
              // Continue parsing GET args
              break;
           case HttpRequestPost:
              // Continue parsing POST args
           default: throw std::invalid_argument("Missing GET or POST");
       }
   }

   void Run()
   {

   }

private:
   CommandLineParser m_oCliParser;
   HttpRequestMethod m_eCommand;

   void printGeneralUsage(){}
};
