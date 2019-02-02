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

#include "HttpRequest.h"

class HttpResponse
{
public:
   HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode,
                 const std::string & in_krsReasonPhrase );
   HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode,
                 const std::string & in_krsReasonPhrase, const HttpContentType & in_kreContentType,
                 const std::initializer_list<HttpHeaders::Headers::value_type>& in_kroMessageHeaders );

   void SetContentType( const HttpContentType& in_kreContentType );
   void SetMessageHeader( const std::string& in_krsFeildName, const std::string& in_krsFeildValue );
   void AppendMessageBody( const std::string & in_krsToAdd );

   const HttpVersion&     GetVersion() const { return m_eVersion; }
   const HttpStatus&      GetStatusCode() const { return m_eStatusCode; }
   const std::string&     GetPhrase() const { return m_sReasonPhrase; }
   const HttpContentType& GetContentType() const { return m_eContentType; }
   const std::string&     GetBody() const { return m_sBody; }

   std::string GetStatusLine() const;
   std::string GetHeaders() const;
   std::string GetWireFormat() const;

private:
   HttpVersion m_eVersion;
   HttpStatus m_eStatusCode;
   std::string m_sReasonPhrase;

   // Optional
   HttpContentType m_eContentType;
   HttpHeaders::Headers m_oHeaders;
   std::string m_sBody;
};

class HttpResponseParser
{
public:
   HttpResponseParser( const std::string& in_krsRequest ) : m_sResponseToParse( in_krsRequest ) { }

   HttpResponse GetHttpResponse() const;

   static HttpStatus STATIC_ParseForStatus( const std::string& in_krsRequest );
   static std::string STATIC_ParseForReasonPhrase( const std::string& in_krsRequest );

   static void STATIC_AppenedParsedHeaders( HttpResponse & io_roRequest, const std::string & in_krsRequest );

private:
   std::string m_sResponseToParse;
};

class HttpResponseParserAdvance
{
public:
   HttpResponseParserAdvance() = default;

   bool AppendResponseData( const std::string& in_krsData );
   HttpResponse GetHttpResponse() const;

private:
   std::string m_sHttpHeader;
   std::string m_sResponseBody;
};
