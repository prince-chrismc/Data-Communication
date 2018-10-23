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

#include "IconServlet.h"
#include <fstream>

IconServlet::IconServlet( const std::string& png_path )
{
   if( png_path.substr( png_path.length() - 4 ) != ".png" ) throw std::invalid_argument( "This server only supports PNG format icons!" );

   std::ifstream fileReader( png_path, std::ios::in | std::ios::binary | std::ios::ate );
   if( !fileReader ) { throw std::invalid_argument( "Unable to use file specified for favicon" ); }

   const size_t size = fileReader.tellg();
   m_IconBytes.resize( size + 1, '\0' ); // construct string
   fileReader.seekg( 0 ); // rewind
   fileReader.read( m_IconBytes.data(), size );
}

HttpResponse IconServlet::HandleRequest( const HttpRequest& request ) const noexcept
{
   HttpResponse oResponse( HttpVersion10, HttpStatusOk, "OK", HttpContentPng, {} );
   oResponse.AppendMessageBody( m_IconBytes );
   return oResponse;
}
