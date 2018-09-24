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
#include "Href.h"

#include <stdexcept>
#include <string_view>

class CurlAppController
{
public:
   CurlAppController( int argc, char** argv );

   void Initialize();

   void Run();

private:
   CommandLineParser m_oCliParser;
   HttpRequestMethod m_eCommand;
   bool              m_bVerbose;
   std::vector<std::pair<std::string, std::string>> m_oExtraHeaders;
   Href              m_oHref;
   std::string       m_sBody;

   static void printGeneralUsage();
   static void printGetUsage();
   static void printPostUsage();
   void printUsageGivenArgs();

   void parseGetOptions( CommandLineParser::ArgIterator itor );
   void parsePostOptions( CommandLineParser::ArgIterator itor );

   void parseVerboseOption( CommandLineParser::ArgIterator& itor );
   void parseHeaderOption( CommandLineParser::ArgIterator& itor );
   void parseUrlOption( CommandLineParser::ArgIterator& itor );

   static constexpr std::string_view MISSING_GET_OR_POST = "Missing 'get' or 'post' paramater";
   static constexpr std::string_view MISSING_URL = "Missing 'URL' paramater";
   void moreArgsToRead( CommandLineParser::ArgIterator itor, std::string_view errMsg );
};
