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
#include <exception>

int main( int argc, char** argv )
{
   CurlAppController oApp( argc, argv );

   try
   {
      oApp.Initialize();

      oApp.Run();
   }
   catch( const std::exception& e )
   {
      std::cout << std::endl << "  --> ERROR: " << e.what() << std::endl;
   }

   return 1;

//    CActiveSocket oClient;
//    HttpResponseParserAdvance oResponseParserParser;

//    bool retval = oClient.Initialize();

//    if( retval )
//    {
//       retval = oClient.Open( "www.google.ca", 80 );
//    }

//    if( retval )
//    {
//       HttpRequest oReq( HttpRequestGet, "/", HttpVersion10, "www.google.ca" );
//       oReq.SetContentType( HttpContentHtml );
//       std::string sRawRequest = oReq.GetWireFormat();
//       retval = oClient.Send( (uint8*)sRawRequest.c_str(), sRawRequest.size() );
//    }

//    if( retval )
//    {
//       int32 bytes_rcvd = -1;
//       do
//       {
//          bytes_rcvd = oClient.Receive( 1024 );

//          if( bytes_rcvd <= 0 ) break; // Transmission completed

//       } while( !oResponseParserParser.AppendResponseData(
//          std::string( (const char*)oClient.GetData(), bytes_rcvd ) ) );
//    }

//    HttpResponse oRes = oResponseParserParser.GetHttpResponse();

//    oClient.Close();

//    std::cout << oRes.GetBody();
//    std::cout.flush();
}


/*
General Usage
httpc help
httpc is a curl-like application but supports HTTP protocol only.
Usage:
httpc command [arguments]
The commands are:
get     executes a HTTP GET request and prints the response.
post    executes a HTTP POST request and prints the response.
Other arguments are:
help    prints extremely helpful screen.
Use "httpc help [command]" for more information about a command.

---------------------------------------------------------------------------------------------------

Get Usage
httpc help get
usage: httpc get [ -v ] [ -h key:value ] URL
Get executes a HTTP GET request for a given URL.
-v             Prints the detail of the response such as protocol, status, and headers.
-h key:value   Associates headers to HTTP Request with the format 'key:value'.

---------------------------------------------------------------------------------------------------

Post Usage
httpc help post
usage: httpc post [ -v ] [ -h key:value ] [ -d inline-data ] | [ -f file ] URL
Post executes a HTTP POST request for a given URL with inline data or from file.

-v             Prints the detail of the response such as protocol, status,and headers.
-h key:value   Associates headers to HTTP Request with the format 'key:value'.
-d string      Associates an inline data to the body HTTP POST request.
-f file        Associates the content of a file to the body HTTP POST request.
Either [ -d ] or [ -f ] can be used but not both.

*/
