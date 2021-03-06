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
   HttpResponse( Http::Version version, Http::Status status, const std::string& reason_phrase = "" );
   HttpResponse( Http::Version version, Http::Status status, const std::string& reason_phrase,
                 Http::ContentType content_type, std::initializer_list<Http::Header::Entry> headers );

   bool IsValid() const;

   void SetContentType( Http::ContentType content_type );
   void SetMessageHeader( const std::string& key, const std::string& value );
   bool HasMessageHeader( const std::string& key, const std::string& value = "" );
   void AppendMessageBody( const std::string& data );

   const Http::Version&     GetVersion() const { return m_eVersion; }
   const Http::Status&      GetStatusCode() const { return m_eStatusCode; }
   const std::string&       GetPhrase() const { return m_sReasonPhrase; }
   const Http::ContentType& GetContentType() const { return m_eContentType; }
   const std::string&       GetBody() const { return m_sBody; }

   std::string GetStatusLine() const;
   std::string GetHeaders() const;
   std::string GetWireFormat() const;

   static std::string STATIC_StatusToReasonPhrase( Http::Status status );

private:
   Http::Version m_eVersion;
   Http::Status m_eStatusCode;
   std::string m_sReasonPhrase;

   // Optional
   Http::ContentType m_eContentType;
   Http::Headers m_oHeaders;
   std::string m_sBody;
};

class HttpResponseParser : HttpRequestParser
{
public:
   HttpResponseParser() = default;

   bool AppendResponseData( const std::string& data );
   HttpResponse GetHttpResponse() const;

private:
   static Http::Status STATIC_ParseForStatus( const std::string& request );
   static std::string STATIC_ParseForReasonPhrase( const std::string& request );
};
