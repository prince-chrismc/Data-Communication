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

#pragma once

#include "Constants.h"
#include <string>
#include <unordered_map>

std::string trim( const std::string& str, const std::string& whitespace = " \t" );
std::string reduce( const std::string& str, const std::string& fill = " ", const std::string& whitespace = " \t" );

namespace HttpHeaders
{
   struct comparison {
      bool operator() ( const std::string & s1, const std::string & s2 ) const {
         return std::lexicographical_compare(
            s1.begin(), s1.end(),
            s2.begin(), s2.end(),
            []( char c1, char c2 ) {
               return ::tolower( c1 ) < ::tolower( c2 );
            } );
      }
   };

   struct Headers : std::unordered_map<std::string, std::string, std::hash<std::string>, comparison>
   {
      Headers( std::initializer_list<value_type> in_kroMessageHeaders );
      void SetContentType( HttpContentType in_eContentType );
   };

   using Key = Headers::key_type;
   using Value = Headers::mapped_type;

   struct Header : private Headers::value_type
   {
      Header( Headers::value_type::first_type& first, Headers::value_type::second_type second )
         : Headers::value_type( first, second ),
         key( this->first ),
         value( this->second )
      {}

      Header( Headers::value_type val )
         : Headers::value_type( val ),
         key( this->first ),
         value( this->second )
      {}

      const Key& key;
      Value value;
   };

   struct EmplaceResult : private std::pair<Headers::iterator, bool>
   {
      EmplaceResult( std::pair<Headers::iterator, bool> val )
         : std::pair<Headers::iterator, bool>( val ),
         itor( this->first ),
         success( this->second )
      {}

      Headers::iterator::value_type getHeader() const { return *itor; }

      Headers::iterator& itor;
      bool& success;
   };

};

class HttpRequest
{
public:
   HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort );
   HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort,
                const HttpContentType & in_kreContentType, std::initializer_list<HttpHeaders::Headers::value_type> in_kroMessageHeaders );

   bool IsValidRequest() const;

   void SetContentType( const HttpContentType& in_kreContentType );
   void SetMessageHeader( const std::string& in_krsFeildName, const std::string& in_krsFeildValue );
   void AppendMessageBody( const std::string & in_krsToAdd );

   const HttpRequestMethod& GetMethod() const { return m_eMethod; }
   const std::string&       GetUri() const { return m_sRequestUri; }
   const HttpVersion&       GetVersion() const { return m_eVersion; }
   const std::string&       GetHostAndPort() const { return m_sHostAndPort; }
   const HttpContentType&   GetContentType() const { return m_eContentType; }
   const std::string&       GetBody() const { return m_sBody; }

   std::string GetRequestLine() const;
   std::string GetHeaders() const;
   std::string GetWireFormat() const;

   static std::string STATIC_MethodAsString( const HttpRequestMethod& in_kreMethod );
   static std::string STATIC_VersionAsString( const HttpVersion& in_kreVersion );
   static std::string STATIC_ContentTypeAsString( const HttpContentType& in_kreContentType );
   static std::string STATIC_ContentLengthToString( size_t in_ullContentLength );

private:
   HttpRequestMethod m_eMethod;
   std::string m_sRequestUri;
   HttpVersion m_eVersion;
   std::string m_sHostAndPort;

   // Optional
   HttpContentType m_eContentType;
   HttpHeaders::Headers m_oHeaders;
   std::string m_sBody;

};

class HttpRequestParser
{
public:
   HttpRequestParser( const std::string& in_krsRequest ) : m_sRequestToParse( in_krsRequest ) { }

   HttpRequest GetHttpRequest() const;

   static HttpRequestMethod STATIC_ParseForMethod( const std::string& in_krsRequest );
   static std::string STATIC_ParseForRequestUri( const std::string& in_krsRequest );
   static HttpVersion STATIC_ParseForVersion( const std::string& in_krsRequest );
   static std::string STATIC_ParseForHostAndPort( const std::string& in_krsRequest );
   static HttpContentType STATIC_ParseForContentType( const std::string& in_krsRequest );
   static std::string STATIC_ParseForBody( const std::string& in_krsRequest );

   static void STATIC_AppenedParsedHeaders( HttpRequest& io_roRequest, const std::string& in_krsRequest );


private:
   std::string m_sRequestToParse;
};

class HttpRequestParserAdvance
{
public:
   HttpRequestParserAdvance() = default;

   bool AppendRequestData( const std::string& in_krsData );
   HttpRequest GetHttpRequest() const;

   static size_t STATIC_ParseForContentLength( const std::string& in_krsHttpHeader );
   static bool STATIC_IsHeaderComplete( const std::string& in_krsHttpHeader );
   static bool STATIC_AppendData( const std::string& in_krsData, std::string& io_krsHttpHeader, std::string& io_krsHttpBody );

private:
   std::string m_sHttpHeader;
   std::string m_sRequestBody;
};
