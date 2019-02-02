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
Connection: ep-Alive:
Content-Type: application/json;charset=UTF-8
Content-Length: 286

*/

#define CRLF "\r\n"
#define HTTP_CONTENT_LENGTH "Content-Length"

//---------------------------------------------------------------------------------------------------------------------
HttpResponse::HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode,
                            const std::string & in_krsReasonPhrase ) :
   HttpResponse( in_kreVersion, in_kreStatusCode, in_krsReasonPhrase, HttpContentInvalid,
                 {
                   ( in_kreVersion == HttpVersion11 ) ? HttpHeaders::Headers::value_type{ "Connection" , "keep-alive" } :
                   ( in_kreVersion == HttpVersion10 ) ? HttpHeaders::Headers::value_type{ "Connection" ,"closed" } :
                     HttpHeaders::Headers::value_type{ "", "" },
                   { "Cache-Control",  "no-cache" },
                   { "Accept",  "*/*" },
                   { "Accept-Encoding", "deflate" }
                 }
   )
{
}

//---------------------------------------------------------------------------------------------------------------------
HttpResponse::HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode,
                            const std::string & in_krsReasonPhrase, const HttpContentType & in_kreContentType,
                            const std::initializer_list<HttpHeaders::Headers::value_type>& in_kroMessageHeaders ) :
   m_eVersion( in_kreVersion ),
   m_eStatusCode( in_kreStatusCode ),
   m_sReasonPhrase( reduce( in_krsReasonPhrase, "", CRLF ) ),
   m_eContentType( HttpContentInvalid ),
   m_oHeaders( in_kroMessageHeaders )
{
   SetContentType( in_kreContentType );
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::SetContentType( const HttpContentType & in_kreContentType )
{
   m_eContentType = in_kreContentType;
   m_oHeaders.SetContentType( m_eContentType );
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::SetMessageHeader( const std::string & in_krsFeildName, const std::string & in_krsFeildValue )
{
   if( in_krsFeildName.empty() || in_krsFeildValue.empty() ) return;

   const HttpHeaders::EmplaceResult retval = m_oHeaders.emplace( reduce( in_krsFeildName, "-" ), reduce( in_krsFeildValue ) );

   if( !retval.success ) // already exists
   {
      HttpHeaders::Header existingHeader( *retval.itor );
      existingHeader.value = in_krsFeildValue;
      //m_oHeaders.at( in_krsFeildName ) = in_krsFeildValue;
   }
}

//---------------------------------------------------------------------------------------------------------------------
void HttpResponse::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd );
   SetMessageHeader( HTTP_CONTENT_LENGTH, std::to_string( m_sBody.length() ) );
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
HttpResponse HttpResponseParser::GetHttpResponse() const
{
   HttpResponse oResponse( HttpRequestParser::STATIC_ParseForVersion( m_sResponseToParse ),
                           STATIC_ParseForStatus( m_sResponseToParse ),
                           STATIC_ParseForReasonPhrase( m_sResponseToParse ),
                           HttpRequestParser::STATIC_ParseForContentType( m_sResponseToParse ),
                           {} );
   STATIC_AppenedParsedHeaders( oResponse, m_sResponseToParse );

   oResponse.AppendMessageBody( HttpRequestParser::STATIC_ParseForBody( m_sResponseToParse ) );

   return oResponse;
}

//---------------------------------------------------------------------------------------------------------------------
HttpStatus HttpResponseParser::STATIC_ParseForStatus( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return HttpStatusInvalid;

   const size_t ulStart = in_krsRequest.find( ' ' ) + sizeof( " " ) - 1;
   const size_t ulEnd = in_krsRequest.find( ' ', ulStart );

   const long long llCode = std::stoull( in_krsRequest.substr( ulStart, ulEnd - ulStart ) );

   return HttpStatus( llCode );
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

void HttpResponseParser::STATIC_AppenedParsedHeaders( HttpResponse & io_roRequest, const std::string & in_krsRequest )
{
   for( std::string sRawHeaders = in_krsRequest.substr( in_krsRequest.find( CRLF ) + sizeof( CRLF ) - 1 );
        sRawHeaders.find( CRLF );
        sRawHeaders = sRawHeaders.substr( sRawHeaders.find( CRLF ) + sizeof( CRLF ) - 1 ) )
   {
      std::string sNextHeader = sRawHeaders.substr( 0, sRawHeaders.find( CRLF ) );
      const size_t iSeperatorIndex = sNextHeader.find( ':' );

      if( iSeperatorIndex != std::string::npos )
      {
         std::string sHeaderKey = sNextHeader.substr( 0, iSeperatorIndex );

         if( sHeaderKey == "Content-Length" || sHeaderKey == "Content-Type" )
            continue; // Skip always formatted hedaers !

         io_roRequest.SetMessageHeader( sHeaderKey, sNextHeader.substr( iSeperatorIndex + 1 ) );
      }
   }
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpResponseParserAdvance
//
//---------------------------------------------------------------------------------------------------------------------
bool HttpResponseParserAdvance::AppendResponseData( const std::string & in_krsData )
{
   return HttpRequestParserAdvance::STATIC_AppendData( in_krsData, m_sHttpHeader, m_sResponseBody );
}

//---------------------------------------------------------------------------------------------------------------------
HttpResponse HttpResponseParserAdvance::GetHttpResponse() const
{
   HttpResponse oResponse( HttpRequestParser::STATIC_ParseForVersion( m_sHttpHeader ),
                           HttpResponseParser::STATIC_ParseForStatus( m_sHttpHeader ),
                           HttpResponseParser::STATIC_ParseForReasonPhrase( m_sHttpHeader ),
                           HttpRequestParser::STATIC_ParseForContentType( m_sHttpHeader ),
                           {} );

   HttpResponseParser::STATIC_AppenedParsedHeaders( oResponse, m_sHttpHeader );

   oResponse.AppendMessageBody( m_sResponseBody );

   return oResponse;
}
