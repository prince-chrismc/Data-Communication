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

#include <string>

struct Href
{
   std::string m_sProtocol;
   std::string m_sHostName;
   unsigned int m_nPortNumber;
   std::string m_sUri;
};

class HrefParser
{
public:
   HrefParser() = default;

   HrefParser& Parse( const std::string& fullUrl );

   const Href& GetHref() const { return oHref; }

   class ParseError : public std::exception
   {
   public:
      ParseError( std::string_view what_arg ) : m_sWhat( what_arg ) {}

      const char* what() const noexcept override { return m_sWhat.data(); }

   private:
      const std::string_view m_sWhat;
   };

private:
   Href oHref;
};