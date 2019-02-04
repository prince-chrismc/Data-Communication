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

#include "HttpRequest.h"
#include  <algorithm>

/*  EXAMPLE REQUEST

GET /x-nmos/node/v1.0/self/ HTTP/1.1
cache-control: no-cache
Postman-Ton: 05197dbd-b271-4d43-8c47-0c3e0e4a9e01
User-Agent: PostmanRuntime/6.3.2
Accept: *\/*
Host: 25.25.34.25:12345
accept-encoding: gzip,deflate
Connection: ep-alive

----------------------------------------------------------------------------

GET /x-nmos/node/v1.0/ HTTP/1.1
Host: 25.25.34.25:12345
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:58.0) Gecko/20100101 Firefox/58.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*\/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip,deflate
Connection: ep-alive
Upgrade-Insecure-Requests: 1

*/

#define CRLF "\r\n"

#define OPTIONS_STRING "OPTIONS"
#define GET_STRING "GET"
#define HEAD_STRING "HEAD"
#define POST_STRING "POST"
#define PUT_STRING "PUT"
#define DELETE_STRING "DELETE"
#define TRACE_STRING "TRACE"
#define CONNECT_STRING "CONNECT"
#define PATCH_STRING "PATCH"

#define HTTP_VERSION_10_STRING "HTTP/1.0"
#define HTTP_VERSION_11_STRING "HTTP/1.1"

#define HTTP_HOST "Host: "
#define HTTP_CONTENT_TYPE "Content-Type"
#define HTTP_CONTENT_LENGTH "Content-Length"
#define HTTP_CONTENT_LENGTH_RAW "Content-Length: "

#define HTTP_BODY_SEPERATOR "\r\n\r\n"
#define SIZE_OF_CRLF ( sizeof( CRLF ) - 1 )
#define SIZE_OF_HTTP_BODY_SEPERATOR ( sizeof( HTTP_BODY_SEPERATOR ) - 1 )

// Thanks to https://stackoverflow.com/a/1798170/8480874
std::string trim( const std::string& str,
                  const std::string& whitespace /*= " \t"*/ )
{
   const auto strBegin = str.find_first_not_of( whitespace );
   if( strBegin == std::string::npos )
      return ""; // no content

   const auto strEnd = str.find_last_not_of( whitespace );
   const auto strRange = strEnd - strBegin + 1;

   return str.substr( strBegin, strRange );
}

std::string reduce( const std::string& str,
                    const std::string& fill /*= " "*/,
                    const std::string& whitespace /*= " \t"*/ )
{
    // trim first
   auto result = trim( str, whitespace );

   // replace sub ranges
   auto beginSpace = result.find_first_of( whitespace );
   while( beginSpace != std::string::npos )
   {
      const auto endSpace = result.find_first_not_of( whitespace, beginSpace );
      const auto range = endSpace - beginSpace;

      result.replace( beginSpace, range, fill );

      const auto newStart = beginSpace + fill.length();
      beginSpace = result.find_first_of( whitespace, newStart );
   }

   return result;
}

bool HttpHeaders::comparison::operator()(const std::string& lhs, const std::string& rhs) const
{
   return std::lexicographical_compare(
      lhs.begin(), lhs.end(),
      rhs.begin(), rhs.end(),
      [](char c1, char c2)
      {
         return ::tolower(c1) < ::tolower(c2);
      });
}

HttpHeaders::Headers::Headers( std::initializer_list<value_type> in_kroMessageHeaders )
   : std::map<std::string, std::string, comparison>( in_kroMessageHeaders )
{
}

void HttpHeaders::Headers::SetContentType( Http::ContentType in_eContentType )
{
   if( in_eContentType != Http::ContentType::Invalid )
   {
      const EmplaceResult retval = emplace( HTTP_CONTENT_TYPE, HttpRequest::STATIC_ContentTypeAsString( in_eContentType ) );

      if( !retval.success ) // already exists
      {
         at( HTTP_CONTENT_TYPE ) = HttpRequest::STATIC_ContentTypeAsString( in_eContentType );
      }
   }
   else // Now invalid
   {
      const auto itor = find( HTTP_CONTENT_TYPE );
      if( itor != std::end( *this ) )
      {
         erase( itor );
      }
   }
}

void HttpHeaders::Headers::SetContentLength(size_t length)
{
   if( length > 0 )
   {
      const EmplaceResult retval = emplace( HTTP_CONTENT_LENGTH, std::to_string( length ) );

      if( !retval.success ) // already exists
      {
         at( HTTP_CONTENT_LENGTH ) = std::to_string( length );
      }
   }
   else // Now invalid
   {
      const auto itor = find( HTTP_CONTENT_LENGTH );
      if( itor != std::end( *this ) )
      {
         erase( itor );
      }
   }
}

HttpRequest::HttpRequest( Http::RequestMethod method, const std::string & in_krsRequestUri,
                          Http::Version version, const std::string & in_krsHostAndPort ) :
   HttpRequest( method, in_krsRequestUri, version, in_krsHostAndPort, Http::ContentType::Invalid,
                {
                   ( version == Http::Version::v11 ) ? HttpHeaders::Headers::value_type{ "Connection" , "keep-alive" } :
                   ( version == Http::Version::v10 ) ? HttpHeaders::Headers::value_type{ "Connection" ,"closed" } :
                     HttpHeaders::Headers::value_type{ "", "" },
                   { "Cache-Control",  "no-cache" },
                   { "Accept",  "*/*" },
                   { "Accept-Encoding", "deflate" }
                }
   )
{
}

HttpRequest::HttpRequest( Http::RequestMethod method, const std::string & in_krsRequestUri,
                          Http::Version version, const std::string & in_krsHostAndPort,
                          Http::ContentType content_type, std::initializer_list<HttpHeaders::Headers::value_type> in_kroMessageHeaders ) :
   m_eMethod( method ),
   m_sRequestUri( in_krsRequestUri ),
   m_eVersion( version ),
   m_eContentType( Http::ContentType::Invalid ),
   m_oHeaders( in_kroMessageHeaders )
{
   SetMessageHeader( "Host", in_krsHostAndPort );
   SetContentType( content_type );
}

bool HttpRequest::IsValidRequest() const
{
   return ( m_eMethod != Http::RequestMethod::Invalid ) || ( !m_sRequestUri.empty() ) || ( m_eVersion != Http::Version:: Invalid ) ;
}

void HttpRequest::SetContentType( Http::ContentType content_type )
{
   m_eContentType = content_type;
   m_oHeaders.SetContentType( m_eContentType );
}

void HttpRequest::SetMessageHeader( const std::string & in_krsFeildName, const std::string & in_krsFeildValue )
{
   if( in_krsFeildName.empty() || in_krsFeildValue.empty() ) return;

   const HttpHeaders::EmplaceResult retval = m_oHeaders.emplace( reduce( in_krsFeildName, "-" ), reduce( in_krsFeildValue ) );

   if( !retval.success ) // already exists
   {
      //HttpHeaders::Header existingHeader( *retval.itor );
      //existingHeader.value = in_krsFeildValue;
      retval.itor->second = in_krsFeildValue;
   }
}

void HttpRequest::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd );
   m_oHeaders.SetContentLength( m_sBody.length() );
}

std::string HttpRequest::GetRequestLine() const
{
   return STATIC_MethodAsString( m_eMethod ) + " " + m_sRequestUri + " " + STATIC_VersionAsString( m_eVersion ) + CRLF;
}

std::string HttpRequest::GetHeaders() const
{
   std::string sCostumHeaders;

   for( auto& sMessageHeader : m_oHeaders )
      sCostumHeaders += ( sMessageHeader.first + ": " + sMessageHeader.second + CRLF );

   return sCostumHeaders;
}

std::string HttpRequest::GetWireFormat() const
{
   return GetRequestLine() + GetHeaders() + CRLF + m_sBody;
}

std::string HttpRequest::STATIC_MethodAsString( Http::RequestMethod method )
{
   switch( method )
   {
   case Http::RequestMethod::Options: return OPTIONS_STRING;
   case Http::RequestMethod::Get: return GET_STRING;
   case Http::RequestMethod::Head: return HEAD_STRING;
   case Http::RequestMethod::Post: return POST_STRING;
   case Http::RequestMethod::Put: return PUT_STRING;
   case Http::RequestMethod::Delete: return DELETE_STRING;
   case Http::RequestMethod::Trace: return TRACE_STRING;
   case Http::RequestMethod::Connect: return CONNECT_STRING;
   case Http::RequestMethod::Patch: return PATCH_STRING;
   default:
      return "";
   }
}

std::string HttpRequest::STATIC_VersionAsString( Http::Version version )
{
   switch( version )
   {
   case Http::Version::v10: return HTTP_VERSION_10_STRING;
   case Http::Version::v11: return HTTP_VERSION_11_STRING;
   default:
      return "";
   }
}

std::string HttpRequest::STATIC_ContentTypeAsString( Http::ContentType content_type )
{
   switch( content_type )
   {
   case Http::ContentType::Text: return "text/plain;";
   case Http::ContentType::Html: return "application/html; text/html;";
   case Http::ContentType::Json: return "application/json; text/json;";
   case Http::ContentType::Yaml: return "application/x-yaml; text/yaml;";
   case Http::ContentType::Xml:  return "text/xml;";
   case Http::ContentType::Gif:  return "image/gif";
   case Http::ContentType::Ico:  return "image/x-icon";
   case Http::ContentType::Png:  return "image/png";
   default: return "";
   }
}

std::string HttpRequest::STATIC_ContentLengthToString( size_t length )
{
   return HTTP_CONTENT_LENGTH + std::to_string( length );
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpRequestParser
//
//---------------------------------------------------------------------------------------------------------------------
HttpRequest HttpRequestParser::GetHttpRequest() const
{
   HttpRequest oRequest( STATIC_ParseForMethod( m_sRequestToParse ),
                         STATIC_ParseForRequestUri( m_sRequestToParse ),
                         STATIC_ParseForVersion( m_sRequestToParse ),
                         STATIC_ParseForHostAndPort( m_sRequestToParse ),
                         STATIC_ParseForContentType( m_sRequestToParse ),
                         {} );

   STATIC_AppenedParsedHeaders( oRequest, m_sRequestToParse );

   oRequest.AppendMessageBody( STATIC_ParseForBody( m_sRequestToParse ) );

   return oRequest;
}

Http::RequestMethod HttpRequestParser::STATIC_ParseForMethod( const std::string & in_krsRequest )
{
   if( in_krsRequest.find( OPTIONS_STRING ) == 0 ) return Http::RequestMethod::Options;
   if( in_krsRequest.find( GET_STRING ) == 0 ) return Http::RequestMethod::Get;
   if( in_krsRequest.find( HEAD_STRING ) == 0 ) return Http::RequestMethod::Head;
   if( in_krsRequest.find( POST_STRING ) == 0 ) return Http::RequestMethod::Post;
   if( in_krsRequest.find( PUT_STRING ) == 0 ) return Http::RequestMethod::Put;
   if( in_krsRequest.find( DELETE_STRING ) == 0 ) return Http::RequestMethod::Delete;
   if( in_krsRequest.find( TRACE_STRING ) == 0 ) return Http::RequestMethod::Trace;
   if( in_krsRequest.find( CONNECT_STRING ) == 0 ) return Http::RequestMethod::Connect;
   if( in_krsRequest.find( PATCH_STRING ) == 0 ) return Http::RequestMethod::Patch;

   return Http::RequestMethod::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForRequestUri( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   std::string sRequestLine = in_krsRequest.substr( 0, in_krsRequest.find( " HTTP" ) );

   size_t ulOffset = HttpRequest::STATIC_MethodAsString( STATIC_ParseForMethod( sRequestLine ) ).size() + 1;

   return sRequestLine.substr( ulOffset );
}

Http::Version HttpRequestParser::STATIC_ParseForVersion( const std::string & in_krsRequest )
{
   if( in_krsRequest.find( HTTP_VERSION_10_STRING ) != std::string::npos ) return Http::Version::v10;
   if( in_krsRequest.find( HTTP_VERSION_11_STRING ) != std::string::npos ) return Http::Version::v11;

   return Http::Version::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForHostAndPort( const std::string & in_krsRequest )
{
   if( !in_krsRequest.empty() )
   {
      size_t ulOffset = in_krsRequest.find( HTTP_HOST ) + sizeof( HTTP_HOST ) - 1;
      size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
      return in_krsRequest.substr( ulOffset, ulEnd - ulOffset );
   }

   return "";
}

Http::ContentType HttpRequestParser::STATIC_ParseForContentType( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return Http::ContentType::Invalid;

   if( in_krsRequest.find( HTTP_CONTENT_TYPE ) == std::string::npos ) return Http::ContentType::Invalid;

   size_t ulOffset = in_krsRequest.find( HTTP_CONTENT_TYPE ) + sizeof( HTTP_CONTENT_TYPE ) - 1;
   size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
   std::string sContentTypeLine = in_krsRequest.substr( ulOffset, ulEnd - ulOffset );

   const size_t ulTextPos = sContentTypeLine.find( "text" );
   const size_t ulHtmlPos = sContentTypeLine.find( "text/html" );
   const size_t ulJsonPos = sContentTypeLine.find( "text/json" );
   const size_t ulHtmlAppPos = sContentTypeLine.find( "application/html" );
   const size_t ulJsonAppPos = sContentTypeLine.find( "application/json" );

   // TODO : Support new format types { PNG, GIF, ICO }

   size_t ulMinPos = std::min( { ulTextPos, ulHtmlPos, ulJsonPos, ulHtmlAppPos, ulJsonAppPos } );

   if( ulMinPos == ulHtmlPos ) return Http::ContentType::Html;
   if( ulMinPos == ulJsonPos ) return Http::ContentType::Json;
   if( ulMinPos == ulHtmlAppPos ) return Http::ContentType::Html;
   if( ulMinPos == ulJsonAppPos ) return Http::ContentType::Json;
   if( ulMinPos == ulTextPos ) return Http::ContentType::Text;

   return Http::ContentType::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForBody( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   size_t ulOffset = in_krsRequest.find( std::string( CRLF ) + CRLF ) + ( sizeof( CRLF ) - 1 ) * 2;

   if( ulOffset == in_krsRequest.size() ) return "";

   return in_krsRequest.substr( ulOffset );
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpRequestParserAdvanced
//
//---------------------------------------------------------------------------------------------------------------------
bool HttpRequestParserAdvance::AppendRequestData( const std::string & in_krsData )
{
   return STATIC_AppendData( in_krsData, m_sHttpHeader, m_sRequestBody );
}

HttpRequest HttpRequestParserAdvance::GetHttpRequest() const
{
   if( m_sHttpHeader.empty() ) return { Http::RequestMethod::Invalid, "", Http::Version::Invalid, "" }; // No data has been obtained!

   HttpRequest oRequest( HttpRequestParser::STATIC_ParseForMethod( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForRequestUri( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForVersion( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForHostAndPort( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForContentType( m_sHttpHeader ),
                         {} );

   HttpRequestParser::STATIC_AppenedParsedHeaders( oRequest, m_sHttpHeader );

   oRequest.AppendMessageBody( m_sRequestBody );

   return oRequest;
}

size_t HttpRequestParserAdvance::STATIC_ParseForContentLength( const std::string & in_krsHttpHeader )
{
   if( in_krsHttpHeader.empty() ) return 0;

   size_t sizeStart = in_krsHttpHeader.find( HTTP_CONTENT_LENGTH_RAW );

   if( sizeStart == std::string::npos ) return 0;

   sizeStart += sizeof( HTTP_CONTENT_LENGTH_RAW ) - 1;
   size_t sizeEnd = in_krsHttpHeader.find( CRLF, sizeStart );

   std::string sContentLength = in_krsHttpHeader.substr( sizeStart, sizeEnd - sizeStart );

   if( sContentLength.length() && sContentLength.find_first_not_of( "0123456789" ) == std::string::npos )
   {
      return std::stoull( sContentLength );
   }

   return 0;
}

bool HttpRequestParserAdvance::STATIC_IsHeaderComplete( const std::string & in_krsHttpHeader )
{
   if( in_krsHttpHeader.size() > SIZE_OF_HTTP_BODY_SEPERATOR )
   {
      return ( in_krsHttpHeader.substr( in_krsHttpHeader.size() - SIZE_OF_HTTP_BODY_SEPERATOR ) == HTTP_BODY_SEPERATOR );
   }

   return false;
}

bool HttpRequestParserAdvance::STATIC_AppendData( const std::string & in_krsData, std::string & io_krsHttpHeader, std::string & io_krsHttpBody )
{
   if( in_krsData.empty() ) return true;

   if( STATIC_IsHeaderComplete( io_krsHttpHeader ) )
   {
      io_krsHttpBody.append( in_krsData );
      return( io_krsHttpBody.size() == STATIC_ParseForContentLength( io_krsHttpHeader ) );
   }

   size_t ullSeperatorIndex = in_krsData.find( HTTP_BODY_SEPERATOR );
   if( ullSeperatorIndex == std::string::npos )
   {
      io_krsHttpHeader.append( in_krsData );
   }
   else
   {
      io_krsHttpHeader.append( in_krsData.substr( 0, ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
      io_krsHttpBody.append( in_krsData.substr( ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
   }
   return STATIC_IsHeaderComplete( io_krsHttpHeader ) && ( io_krsHttpBody.size() == STATIC_ParseForContentLength( io_krsHttpHeader ) );
}
