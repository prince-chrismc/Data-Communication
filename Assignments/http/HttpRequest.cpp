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
#include <cctype>

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

#define HTTP_VERSION_PREFIX " HTTP/"
#define HTTP_VERSION_10_STRING "HTTP/1.0"
#define HTTP_VERSION_11_STRING "HTTP/1.1"

#define HTTP_HOST "Host"
#define HTTP_CONTENT_TYPE "Content-Type"
#define HTTP_CONTENT_LENGTH "Content-Length"

#define HTTP_HOST_RAW "Host: "
#define HTTP_CONTENT_TYPE_RAW "Content-Type: "
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

//---------------------------------------------------------------------------------------------------------------------
//
// Http::Headers
//
//---------------------------------------------------------------------------------------------------------------------
bool Http::Comparison::operator()( const std::string& lhs, const std::string& rhs ) const
{
   return std::lexicographical_compare(
      lhs.begin(), lhs.end(),
      rhs.begin(), rhs.end(),
      []( char c1, char c2 )
      {
         return ::tolower( c1 ) < ::tolower( c2 );
      } );
}

Http::Headers::Headers( std::initializer_list<value_type> headers )
   : std::map<std::string, std::string, Comparison>( headers )
{
}

void Http::Headers::SetContentType( ContentType in_eContentType )
{
   if( in_eContentType != ContentType::Invalid )
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

void Http::Headers::SetContentLength( size_t length )
{
   const EmplaceResult retval = emplace( HTTP_CONTENT_LENGTH, std::to_string( length ) );

   if( !retval.success ) // already exists
   {
      at( HTTP_CONTENT_LENGTH ) = std::to_string( length );
   }
}

std::string Http::Headers::AsString() const
{
   std::string buffer;

   for( ConstHeader header : *this )
      buffer += ( header.key + ": " + header.value + CRLF );

   return buffer;
}

std::string Http::Headers::FormatHeaderKey( const std::string& in_krsHeaderKey )
{
   if( in_krsHeaderKey.empty() )
      return in_krsHeaderKey;

   auto key = reduce( in_krsHeaderKey, "-" );
   std::transform( key.begin(), key.end(), key.begin(), []( char c )->char { return static_cast<char>( std::tolower( c ) ); } );

   key[ 0 ] = static_cast<char>( std::toupper( key[ 0 ] ) );

   auto beginSpace = key.find_first_of( '-' );
   while( beginSpace != std::string::npos )
   {
      beginSpace += 1;
      if( beginSpace <= key.length() )
      {
         key[ beginSpace ] = static_cast<char>( std::toupper( key[ beginSpace ] ) );
      }

      beginSpace = key.find_first_of( '-', beginSpace );
   }

   return key;
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpRequest
//
//---------------------------------------------------------------------------------------------------------------------
HttpRequest::HttpRequest( Http::RequestMethod method, const std::string & uri,
                          Http::Version version, const std::string & host_port ) :
   HttpRequest( method, uri, version, host_port, Http::ContentType::Invalid,
                {
                   ( version == Http::Version::v11 ) ? Http::Header::Entry{ "Connection" , "keep-alive" } :
                   ( version == Http::Version::v10 ) ? Http::Header::Entry{ "Connection" , "closed" } :
                     Http::Header::Entry{ "", "" },
                   { "Cache-Control",  "no-cache" },
                   { "Accept",  "*/*" },
                }
   )
{
}

HttpRequest::HttpRequest( Http::RequestMethod method, const std::string & uri,
                          Http::Version version, const std::string & host_port,
                          Http::ContentType content_type, std::initializer_list<Http::Header::Entry> headers ) :
   m_eMethod( method ),
   m_sRequestUri( uri ),
   m_eVersion( version ),
   m_eContentType( Http::ContentType::Invalid ),
   m_oHeaders( headers )
{
   if( m_eVersion == Http::Version::v11 )
   {
      m_oHeaders.SetContentLength( m_sBody.length() );
   }
   SetMessageHeader( HTTP_HOST, host_port );
   SetContentType( content_type );
}

bool HttpRequest::IsValid() const
{
   return ( m_eMethod != Http::RequestMethod::Invalid ) && ( !m_sRequestUri.empty() ) && ( m_eVersion != Http::Version::Invalid );
}

void HttpRequest::SetContentType( Http::ContentType content_type )
{
   m_eContentType = content_type;
   m_oHeaders.SetContentType( m_eContentType );
}

void HttpRequest::SetMessageHeader( const std::string & key, const std::string & value )
{
   if( key.empty() || value.empty() ) return;

   const Http::EmplaceResult retval = m_oHeaders.emplace( Http::Headers::FormatHeaderKey( key ), reduce( value ) );

   if( !retval.success ) // already exists
   {
      retval.GetHeader().value = value;
   }
}

bool HttpRequest::HasMessageHeader( const std::string& key, const std::string& value )
{
   const auto itor = m_oHeaders.find( key );
   if( itor != std::end( m_oHeaders ) )
   {
      if( !value.empty() )
      {
         return itor->second.find( value ) != std::string::npos;
      }

      return true;
   }

   return false;
}

void HttpRequest::AppendMessageBody( const std::string & data )
{
   m_sBody.append( data );
   m_oHeaders.SetContentLength( m_sBody.length() );
}

std::string HttpRequest::GetRequestLine() const
{
   return STATIC_MethodAsString( m_eMethod ) + " " + m_sRequestUri + " " + STATIC_VersionAsString( m_eVersion ) + CRLF;
}

std::string HttpRequest::GetHeaders() const
{
   return m_oHeaders.AsString();
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
   case Http::ContentType::Xml:  return "text/xml";
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
   if( m_sHttpHeader.empty() ) return { Http::RequestMethod::Invalid, "", Http::Version::Invalid, "" }; // No data has been obtained!

   HttpRequest oRequest( STATIC_ParseForMethod( m_sHttpHeader ),
                         STATIC_ParseForRequestUri( m_sHttpHeader ),
                         STATIC_ParseForVersion( m_sHttpHeader ),
                         STATIC_ParseForHostAndPort( m_sHttpHeader ),
                         STATIC_ParseForContentType( m_sHttpHeader ),
                         {} );

   STATIC_AppenedParsedHeaders( oRequest, m_sHttpHeader );

   oRequest.AppendMessageBody( m_sMessageBody );

   return oRequest;
}

bool HttpRequestParser::AppendRequestData( const std::string & data )
{
   if( data.empty() ) return true;

   if( STATIC_IsHeaderComplete( m_sHttpHeader ) )
   {
      m_sMessageBody.append( data );
      return( m_sMessageBody.size() == STATIC_ParseForContentLength( m_sHttpHeader ) );
   }

   const size_t ullSeperatorIndex = data.find( HTTP_BODY_SEPERATOR );
   if( ullSeperatorIndex == std::string::npos )
   {
      m_sHttpHeader.append( data );
   }
   else
   {
      m_sHttpHeader.append( data.substr( 0, ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
      m_sMessageBody.append( data.substr( ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
   }
   return STATIC_IsHeaderComplete( m_sHttpHeader ) && ( m_sMessageBody.size() == STATIC_ParseForContentLength( m_sHttpHeader ) );
}

Http::RequestMethod HttpRequestParser::STATIC_ParseForMethod( const std::string & request )
{
   if( request.find( OPTIONS_STRING ) == 0 ) return Http::RequestMethod::Options;
   if( request.find( GET_STRING ) == 0 ) return Http::RequestMethod::Get;
   if( request.find( HEAD_STRING ) == 0 ) return Http::RequestMethod::Head;
   if( request.find( POST_STRING ) == 0 ) return Http::RequestMethod::Post;
   if( request.find( PUT_STRING ) == 0 ) return Http::RequestMethod::Put;
   if( request.find( DELETE_STRING ) == 0 ) return Http::RequestMethod::Delete;
   if( request.find( TRACE_STRING ) == 0 ) return Http::RequestMethod::Trace;
   if( request.find( CONNECT_STRING ) == 0 ) return Http::RequestMethod::Connect;
   if( request.find( PATCH_STRING ) == 0 ) return Http::RequestMethod::Patch;

   return Http::RequestMethod::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForRequestUri( const std::string & request )
{
   if( request.empty() ) return "";

   std::string sRequestLine = request.substr( 0, request.find( HTTP_VERSION_PREFIX ) );

   size_t ulOffset = HttpRequest::STATIC_MethodAsString( STATIC_ParseForMethod( sRequestLine ) ).size() + 1;

   return sRequestLine.substr( ulOffset );
}

Http::Version HttpRequestParser::STATIC_ParseForVersion( const std::string & request )
{
   if( request.find( HTTP_VERSION_10_STRING ) != std::string::npos ) return Http::Version::v10;
   if( request.find( HTTP_VERSION_11_STRING ) != std::string::npos ) return Http::Version::v11;

   return Http::Version::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForHostAndPort( const std::string & request )
{
   if( !request.empty() )
   {
      size_t ulOffset = request.find( HTTP_HOST_RAW ) + sizeof( HTTP_HOST_RAW ) - 1;
      size_t ulEnd = request.find( CRLF, ulOffset );
      return request.substr( ulOffset, ulEnd - ulOffset );
   }

   return "";
}

Http::ContentType HttpRequestParser::STATIC_ParseForContentType( const std::string & request )
{
   if( request.empty() ) return Http::ContentType::Invalid;

   if( request.find( HTTP_CONTENT_TYPE ) == std::string::npos ) return Http::ContentType::Invalid;

   size_t ulOffset = request.find( HTTP_CONTENT_TYPE_RAW ) + sizeof( HTTP_CONTENT_TYPE_RAW ) - 1;
   size_t ulEnd = request.find( CRLF, ulOffset );
   std::string sContentTypeLine = request.substr( ulOffset, ulEnd - ulOffset );

   const size_t ulTextPos = sContentTypeLine.find( "text" );
   const size_t ulHtmlPos = sContentTypeLine.find( "text/html" );
   const size_t ulJsonPos = sContentTypeLine.find( "text/json" );
   const size_t ulHtmlAppPos = sContentTypeLine.find( "application/html" );
   const size_t ulJsonAppPos = sContentTypeLine.find( "application/json" );
   const size_t ulYamlPos = sContentTypeLine.find( "text/yaml" );
   const size_t ulYamlAppPos = sContentTypeLine.find( "application/x-yaml" );
   const size_t ulXmlPos = sContentTypeLine.find( "text/xml" );
   const size_t ulGifPos = sContentTypeLine.find( "image/gif" );
   const size_t ulIcoPos = sContentTypeLine.find( "image/x-icon" );
   const size_t ulPngPos = sContentTypeLine.find( "image/png" );

   const size_t ulMinPos = std::min( { ulTextPos, ulHtmlPos, ulJsonPos, ulHtmlAppPos, ulJsonAppPos, ulYamlPos, ulYamlAppPos, ulXmlPos, ulGifPos, ulIcoPos, ulPngPos } );

   if( ulMinPos == ulHtmlPos ) return Http::ContentType::Html;
   if( ulMinPos == ulJsonPos ) return Http::ContentType::Json;
   if( ulMinPos == ulHtmlAppPos ) return Http::ContentType::Html;
   if( ulMinPos == ulJsonAppPos ) return Http::ContentType::Json;
   if( ulMinPos == ulYamlPos ) return Http::ContentType::Yaml;
   if( ulMinPos == ulYamlAppPos ) return Http::ContentType::Yaml;
   if( ulMinPos == ulXmlPos ) return Http::ContentType::Xml;
   if( ulMinPos == ulGifPos ) return Http::ContentType::Gif;
   if( ulMinPos == ulIcoPos ) return Http::ContentType::Ico;
   if( ulMinPos == ulPngPos ) return Http::ContentType::Png;
   if( ulMinPos == ulTextPos ) return Http::ContentType::Text;

   return Http::ContentType::Invalid;
}

std::string HttpRequestParser::STATIC_ParseForBody( const std::string & request )
{
   if( request.empty() )
      return "";

   const size_t ulOffset = request.find( std::string( CRLF ) + CRLF ) + ( sizeof( CRLF ) - 1 ) * 2;
   if( ulOffset == request.size() )
      return "";

   return request.substr( ulOffset );
}

size_t HttpRequestParser::STATIC_ParseForContentLength( const std::string & headers_buffer )
{
   if( headers_buffer.empty() )
      return 0;

   size_t sizeStart = headers_buffer.find( HTTP_CONTENT_LENGTH_RAW );

   if( sizeStart == std::string::npos )
      return 0;

   sizeStart += sizeof( HTTP_CONTENT_LENGTH_RAW ) - 1;
   const size_t sizeEnd = headers_buffer.find( CRLF, sizeStart );

   std::string sContentLength = headers_buffer.substr( sizeStart, sizeEnd - sizeStart );

   if( sContentLength.length() && sContentLength.find_first_not_of( "0123456789" ) == std::string::npos )
      return std::stoull( sContentLength );

   return 0;
}

bool HttpRequestParser::STATIC_IsHeaderComplete( const std::string & headers_buffer )
{
   if( headers_buffer.size() > SIZE_OF_HTTP_BODY_SEPERATOR )
   {
      return ( headers_buffer.substr( headers_buffer.size() - SIZE_OF_HTTP_BODY_SEPERATOR ) == HTTP_BODY_SEPERATOR );
   }

   return false;
}
