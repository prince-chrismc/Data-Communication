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

#include "Href.h"
#include <algorithm>

constexpr auto PROTOCOL_ENDING = "://";
int constexpr length( const char* str ) { return *str ? 1 + length( str + 1 ) : 0; }

HrefParser& HrefParser::Parse( const std::string& fullUrl )
{
   // Some Lovely Defaults
   oHref.m_sProtocol = "http";
   oHref.m_nPortNumber = 80;
   oHref.m_sHostName = "www.google.ca";
   oHref.m_sUri = "/";

   size_t ulProtocol = fullUrl.find( PROTOCOL_ENDING );
   if( ulProtocol == std::string::npos ) throw ParseError( "URL missing protocol seperator" );

   oHref.m_sProtocol = fullUrl.substr( 0, ulProtocol );

   ulProtocol += length( PROTOCOL_ENDING );

   size_t ulColumn = fullUrl.find( ':', ulProtocol );
   size_t ulSlash = fullUrl.find( '/', ulProtocol );

   size_t ulEndOfHostName = std::min( ulColumn, ulSlash );
   oHref.m_sHostName = fullUrl.substr( ulProtocol, ulEndOfHostName - ulProtocol );
   if( oHref.m_sHostName.empty() ) throw ParseError( "Unable to determine host name or IP addr" );

   if( ulColumn != std::string::npos && ulSlash != std::string::npos )
      oHref.m_nPortNumber = std::stoul( fullUrl.substr( ulColumn, ulSlash - ulColumn ) );

   if( ulEndOfHostName != std::string::npos )
      oHref.m_sUri = fullUrl.substr( ulEndOfHostName );

   return *this;
}
