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
#include <string_view>

using namespace std::string_view_literals;

constexpr auto PROTOCOL_ENDING = "://"sv;

HrefParser& HrefParser::Parse( const std::string& fullUrl )
{
   size_t ulProtocol = fullUrl.find( PROTOCOL_ENDING );
   if( ulProtocol == std::string::npos )
      throw ParseError( "URL missing protocol seperator" );

   m_Href.m_sProtocol = fullUrl.substr( 0, ulProtocol );

   ulProtocol += PROTOCOL_ENDING.length();

   const size_t ulColumn = fullUrl.find( ':', ulProtocol );
   const size_t ulSlash = fullUrl.find( '/', ulProtocol );

   const size_t ulEndOfHostName = std::min( ulColumn, ulSlash );
   m_Href.m_sHostName = fullUrl.substr( ulProtocol, ulEndOfHostName - ulProtocol );

   if( m_Href.m_sHostName.empty() )
      throw ParseError( "Unable to determine host name or IP addr" );

   if( ulColumn != std::string::npos && ulSlash != std::string::npos )
      m_Href.m_nPortNumber = static_cast<unsigned short>( std::stoul( fullUrl.substr( ulColumn + 1, ulSlash - ulColumn - 1 ) ) );

   if( ulEndOfHostName != std::string::npos )
      m_Href.m_sUri = fullUrl.substr( ulSlash );

   return *this;
}
