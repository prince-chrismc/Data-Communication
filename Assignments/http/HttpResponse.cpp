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

#include "HttpResponse.h"

/*

HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: text/html
Connection: Closed

------------------------------------------------------

HTTP/1.1 200 OK
Cache-Control: no-cache
Server: libnhttpd
Date: Wed Jul 4 15:38:53 2012
Connection: keep-Alive:
Content-Type: application/json;charset=UTF-8
Content-Length: 286

*/

#define CRLF "\r\n"

//---------------------------------------------------------------------------------------------------------------------
HttpResponse::HttpResponse( Http::Version version, Http::Status status, const std::string & in_krsReasonPhrase ) :
   HttpResponse( version, status, in_krsReasonPhrase, Http::ContentType::Invalid,
                 {
                   ( version == Http::Version::v11 ) ? Http::Headers::value_type{ "Connection" , "keep-alive" } :
                   ( version == Http::Version::v10 ) ? Http::Headers::value_type{ "Connection" ,"closed" } :
                     Http::Headers::value_type{ "", "" },
                   { "Cache-Control",  "no-cache" },
                   { "Accept",  "*/*" },
                   { "Accept-Encoding", "deflate" }
                 }
   )
{
}

//---------------------------------------------------------------------------------------------------------------------
HttpResponse::HttpResponse( Http::Version version, Http::Status status, const std::string & in_krsReasonPhrase,
                            Http::ContentType content_type, std::initializer_list<Http::Headers::value_type> in_kroMessageHeaders ) :
   m_eVersion( version ),
   m_eStatusCode( status ),
   m_sReasonPhrase( reduce( in_krsReasonPhrase, "", CRLF ) ),
   m_eContentType( Http::ContentType::Invalid ),
   m_oHeaders( in_kroMessageHeaders )
{
   SetContentType( content_type );
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::SetContentType( Http::ContentType content_type )
{
   m_eContentType = content_type;
   m_oHeaders.SetContentType( m_eContentType );
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::SetMessageHeader( const std::string & in_krsFeildName, const std::string & in_krsFeildValue )
{
   if( in_krsFeildName.empty() || in_krsFeildValue.empty() ) return;

   const Http::EmplaceResult retval = m_oHeaders.emplace( Http::Headers::FormatHeaderKey( in_krsFeildName ), reduce( in_krsFeildValue ) );

   if( !retval.success ) // already exists
   {
      Http::Header existingHeader( *retval.itor );
      existingHeader.value = in_krsFeildValue;
   }
}

//---------------------------------------------------------------------------------------------------------------------
bool HttpResponse::HasMessageHeader(const std::string& in_krsFeildName, const std::string& in_krsFeildValue)
{
   auto itor = m_oHeaders.find(in_krsFeildName);
   if (itor != std::end(m_oHeaders))
   {
      if (!in_krsFeildValue.empty())
      {
         return itor->second.find(in_krsFeildValue) != std::string::npos;
      }

      return true;
   }

   return false;
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd );
   m_oHeaders.SetContentLength( m_sBody.length() );
}

//---------------------------------------------------------------------------------------------------------------------
std::string HttpResponse::GetStatusLine() const
{
   return HttpRequest::STATIC_VersionAsString( m_eVersion ) + " " + std::to_string( static_cast<unsigned long long>( m_eStatusCode ) ) + " " +
      m_sReasonPhrase + CRLF;
}

//---------------------------------------------------------------------------------------------------------------------
std::string HttpResponse::GetHeaders() const
{
   std::string sCostumHeaders;

   for( auto& sMessageHeader : m_oHeaders )
      sCostumHeaders += ( sMessageHeader.first + ": " + sMessageHeader.second + CRLF );

   return sCostumHeaders;
}

//---------------------------------------------------------------------------------------------------------------------
std::string HttpResponse::GetWireFormat() const
{
   return GetStatusLine() + GetHeaders() + CRLF + m_sBody;
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpResponseParser
//
//---------------------------------------------------------------------------------------------------------------------
bool HttpResponseParser::AppendResponseData( const std::string & in_krsData )
{
   return HttpRequestParser::STATIC_AppendData( in_krsData, m_sHttpHeader, m_sResponseBody );
}

//---------------------------------------------------------------------------------------------------------------------
Http::Status HttpResponseParser::STATIC_ParseForStatus( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return Http::Status::Invalid;

   const size_t ulStart = in_krsRequest.find( ' ' ) + sizeof( " " ) - 1;
   const size_t ulEnd = in_krsRequest.find( ' ', ulStart );

   const long long llCode = std::stoull( in_krsRequest.substr( ulStart, ulEnd - ulStart ) );

   return Http::Status( llCode );
}

//---------------------------------------------------------------------------------------------------------------------
std::string HttpResponseParser::STATIC_ParseForReasonPhrase( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   size_t ulOffset = in_krsRequest.find( ' ' ) + 1;
   ulOffset = in_krsRequest.find( ' ', ulOffset ) + 1;
   const size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
   return in_krsRequest.substr( ulOffset, ulEnd - ulOffset );
}

//---------------------------------------------------------------------------------------------------------------------
HttpResponse HttpResponseParser::GetHttpResponse() const
{
   HttpResponse oResponse( HttpRequestParser::STATIC_ParseForVersion( m_sHttpHeader ),
                           STATIC_ParseForStatus( m_sHttpHeader ),
                           STATIC_ParseForReasonPhrase( m_sHttpHeader ),
                           HttpRequestParser::STATIC_ParseForContentType( m_sHttpHeader ),
                           {} );

   HttpRequestParser::STATIC_AppenedParsedHeaders( oResponse, m_sHttpHeader );

   oResponse.AppendMessageBody( m_sResponseBody );

   return oResponse;
}
