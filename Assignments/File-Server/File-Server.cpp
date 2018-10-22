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

#include "HttpServer.h"
#include <iostream>
#include <exception>
#include <filesystem>
#include <fstream>

using namespace std::chrono_literals;

class IconServlet : public HttpServlet
{
public:
   IconServlet( const std::string& png_path )
   {
      if( png_path.substr( png_path.length() - 4 ) != ".png" ) throw std::invalid_argument( "This server only supports PNG format icons!" );

      std::ifstream fileReader( png_path, std::ios::in | std::ios::binary | std::ios::ate );
      if( !fileReader ) { throw std::invalid_argument( "Unable to use file specified for favicon" ); }

      const size_t size = fileReader.tellg();
      m_IconBytes.resize( size + 1, '\0' ); // construct string
      fileReader.seekg( 0 ); // rewind
      fileReader.read( m_IconBytes.data(), size );
   }

   HttpResponse HandleRequest( const HttpRequest& request ) const noexcept override
   {
      HttpResponse oResponse( HttpVersion10, HttpStatusOk, "OK", HttpContentPng, {} );
      oResponse.AppendMessageBody( m_IconBytes );
      return oResponse;
   }

private:
   std::string m_IconBytes;
};

class FileServlet : public HttpServlet
{
public:
   FileServlet( std::string path ) : m_Path( path )
   {
      if( !std::filesystem::is_directory( m_Path ) ) throw std::logic_error( "File exploration must happen from a directory!" );
   }

   HttpResponse HandleRequest( const HttpRequest& request ) const noexcept override
   {
      switch(request.GetMethod())
      {
      case HttpRequestGet:
         return HandleGetRequest( request );
      case HttpRequestPost:
         return HandlePostRequest( request );
      default: break;
      }

      return{ HttpVersion10, HttpStatusMethodNotAllowed, "SERVER ONLY ACCEPTS GET AND POST METHODS" };
   }

   HttpResponse HandleGetRequest( const HttpRequest& request ) const noexcept
   {
      std::string sRequestedItem = request.GetUri().substr( 1 );

      if( sRequestedItem.length() && sRequestedItem.at(0) == '.')
         return{ HttpVersion10, HttpStatusForbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

      const std::filesystem::path oRequested = m_Path / sRequestedItem;

      if( !std::filesystem::exists( oRequested ) )
         return{ HttpVersion10, HttpStatusNotFound, "NOT FOUND" };

      if( std::filesystem::is_directory( oRequested ) )
         return HandleDirectoryRequest( oRequested );

      if( std::filesystem::is_regular_file( oRequested ) )
         return HandleFileRequest( oRequested );

      return{ HttpVersion10, HttpStatusNotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
   }

   HttpResponse HandleDirectoryRequest( const std::filesystem::path& requested ) const noexcept
   {
      HttpResponse oResponse( HttpVersion10, HttpStatusOk, "OK" );

      oResponse.SetContentType( HttpContentText );

      oResponse.AppendMessageBody( "Directory: " + std::filesystem::canonical( requested ).string() + "\r\n" );

      for( auto& oEntry : std::filesystem::directory_iterator( requested ) )
      {
         if( oEntry.is_directory() )
            oResponse.AppendMessageBody( "   - " + oEntry.path().filename().string() + "/\r\n" );
         else if( oEntry.is_regular_file() )
            oResponse.AppendMessageBody( "   - " + oEntry.path().filename().string() + "\r\n" );
      }

      return oResponse;
   }

   HttpResponse HandleFileRequest( const std::filesystem::path& requested ) const noexcept
   {
      HttpResponse oResponse( HttpVersion10, HttpStatusOk, "OK" );
      oResponse.SetContentType( FileExtensionToContentType(requested) );
      oResponse.AddMessageHeader( "Content-Disposition", "inline" );

      if( oResponse.GetContentType() != HttpContentPng )
            oResponse.AppendMessageBody( "File: " + std::filesystem::canonical( requested ).string() + "\r\n" );

      std::ifstream fileReader( requested.string(), std::ios::in | std::ios::binary | std::ios::ate );
      if( !fileReader ) return{ HttpVersion10, HttpStatusInternalServerError, "COULD NOT LOAD FILE" };

      const size_t size = fileReader.tellg();
      std::string fileBuffer( size, '\0' ); // construct buffer
      fileReader.seekg( 0 ); // rewind
      fileReader.read( fileBuffer.data(), size );

      oResponse.AppendMessageBody( fileBuffer );

      return oResponse;
   }

   HttpContentType FileExtensionToContentType( const std::filesystem::path& requested ) const noexcept
   {
      if( !std::filesystem::is_regular_file( requested ) )
         return HttpContentInvalid;

      if( ! requested.has_extension() )
         return HttpContentText;

      if( requested.filename().string().find( ".vcxproj") != std::string::npos )
         return HttpContentXml;

      if( requested.extension() == ".html" )
         return HttpContentHtml;

      if( requested.extension() == ".json" )
         return HttpContentJson;

      if( requested.extension() == ".yml" )
         return HttpContentYaml;

      if( requested.extension() == ".png" )
         return HttpContentPng;

      return HttpContentText;
   }


   HttpResponse HandlePostRequest( const HttpRequest& request ) const noexcept
   {
      std::string sRequestedItem = request.GetUri().substr( 1 );

      if( sRequestedItem.length() && sRequestedItem.at(0) == '.')
         return{ HttpVersion10, HttpStatusForbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

      const std::filesystem::path oRequested = m_Path / sRequestedItem;

      if( !std::filesystem::exists( oRequested ) )
         return CreateItem( oRequested );

      if( std::filesystem::is_directory( oRequested ) )
         return{ HttpVersion10, HttpStatusBadRequest, "CAN NOT CREATE EXISTING DIRECTORIES" };

      if( std::filesystem::is_regular_file( oRequested ) )
         return WriteFile( oRequested, request.GetBody() );

      return{ HttpVersion10, HttpStatusNotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
   }

   HttpResponse CreateItem( const std::filesystem::path& requested ) const noexcept
   {
      if( ! requested.has_extension() )
         return CreateDirectory(requested);
      else
         return CreateFile(requested);
   }

   HttpResponse CreateFile( const std::filesystem::path& requested ) const noexcept
   {
      return{ HttpVersion10, HttpStatusNotImplemented, "CREATE FILES NOT IMPLEMENTED" };
   }

   HttpResponse CreateDirectory( const std::filesystem::path& requested ) const noexcept
   {
      return{ HttpVersion10, HttpStatusNotImplemented, "CREATE DIRECTORIES NOT IMPLEMENTED" };
   }

   HttpResponse WriteFile( const std::filesystem::path& requested,
                           const std::string& content ) const noexcept
   {
      return{ HttpVersion10, HttpStatusNotImplemented, "WRITE FILE NOT IMPLEMENTED" };
   }

private:
   const std::filesystem::path m_Path;
};

int main( int argc, char** argv )
{
   try
   {
      // oApp.Initialize();

      // oApp.Run();

      HttpServer oServer( HttpVersion10 );
      std::unique_ptr<FileServlet> oFileExplorer = std::make_unique<FileServlet>( ".." );
      std::unique_ptr<IconServlet> oFavicon = std::make_unique<IconServlet>( argv[ 1 ] );

      oServer.RegisterServlet( "/", oFileExplorer.get() );
      oServer.RegisterServlet( "/favicon.ico", oFavicon.get() );
      //oServer.RegisterServlet( "/test", nullptr );
      //oServer.RegisterServlet( "/test/123", nullptr );

      oServer.Launch( "127.0.0.1", 8080 );

      std::this_thread::sleep_for( 1h );

      oServer.Close();
   }
   catch( const std::exception& e )
   {
      std::cout << std::endl << "  --> ERROR: " << e.what() << std::endl;
   }

   std::this_thread::sleep_for( 1s );

   return 1;
}
