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

#include "FileServlet.h"
#include <exception>
#include <fstream>
#include <sstream>

FileServlet::FileServlet( const std::string& path ) : m_Path( path )
{
   if( !std::filesystem::is_directory( m_Path ) ) throw std::logic_error( "File exploration must happen from a directory!" );
}

HttpResponse FileServlet::HandleRequest( const HttpRequest& request ) const noexcept
{
   switch( request.GetMethod() )
   {
   case HttpRequestGet:
      return HandleGetRequest( request );
   case HttpRequestPost:
      return HandlePostRequest( request );
   default:
      break;
   }

   return{ HttpVersion10, HttpStatusMethodNotAllowed, "SERVER ONLY ACCEPTS GET AND POST METHODS" };
}

HttpResponse FileServlet::HandleGetRequest( const HttpRequest& request ) const noexcept
{
   if( request.GetUri().find( "/.." ) != std::string::npos )
      return{ HttpVersion10, HttpStatusForbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

   const std::filesystem::path oRequested = m_Path / request.GetUri().substr( 1 );

   if( !std::filesystem::exists( oRequested ) )
      return{ HttpVersion10, HttpStatusNotFound, "NOT FOUND" };

   if( std::filesystem::is_directory( oRequested ) )
      return HandleDirectoryRequest( oRequested );

   if( std::filesystem::is_regular_file( oRequested ) )
      return HandleFileRequest( oRequested );

   return{ HttpVersion10, HttpStatusNotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
}

HttpResponse FileServlet::HandleDirectoryRequest( const std::filesystem::path& requested ) const noexcept
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

HttpResponse FileServlet::HandleFileRequest( const std::filesystem::path& requested ) const noexcept
{
   HttpResponse oResponse( HttpVersion10, HttpStatusOk, "OK" );
   oResponse.SetContentType( FileExtensionToContentType( requested ) );
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

HttpContentType FileServlet::FileExtensionToContentType( const std::filesystem::path& requested ) const noexcept
{
   if( !std::filesystem::is_regular_file( requested ) )
      return HttpContentInvalid;

   if( !requested.has_extension() )
      return HttpContentText;

   if( requested.filename().string().find( ".vcxproj" ) != std::string::npos )
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


HttpResponse FileServlet::HandlePostRequest( const HttpRequest& request ) const noexcept
{
   if( request.GetUri().find( "/.." ) != std::string::npos )
      return{ HttpVersion10, HttpStatusForbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

   const std::filesystem::path oRequested = m_Path / request.GetUri().substr( 1 );

   if( !std::filesystem::exists( oRequested ) )
      return HandleCreateItemRequest( oRequested, request.GetBody() );

   if( std::filesystem::is_directory( oRequested ) )
      return{ HttpVersion10, HttpStatusBadRequest, "CAN NOT CREATE EXISTING DIRECTORIES" };

   if( std::filesystem::is_regular_file( oRequested ) )
      return HandleWriteFileRequest( oRequested, request.GetBody() );

   return{ HttpVersion10, HttpStatusNotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
}

HttpResponse FileServlet::HandleCreateItemRequest( const std::filesystem::path& requested,
                                                   const std::string& content ) const noexcept
{
   if( !requested.has_extension() )
      return HandleCreateDirectoryRequest( requested );
   else
      return HandleCreateFileRequest( requested, content );
}

HttpResponse FileServlet::HandleCreateFileRequest( const std::filesystem::path& requested,
                                                   const std::string& content ) const noexcept
{
   HttpResponse oResponse( HttpVersion10, HttpStatusConflict, "FAILED TO CREATE FILE" );
   try
   {
      std::ofstream fileWriter( requested.string() );

      if( !fileWriter )
         throw std::runtime_error( "Failed to create file located at: " +
                                   std::filesystem::canonical( requested ).string() + "\r\n" );

      fileWriter << content << std::endl;
      fileWriter.close();

      oResponse = HttpResponse( HttpVersion10, HttpStatusCreated, "CREATED FILE" );
      oResponse.SetContentType( FileExtensionToContentType( requested ) );
      oResponse.AddMessageHeader( "Content-Disposition", "inline" );

      if( oResponse.GetContentType() != HttpContentPng )
         oResponse.AppendMessageBody( "File: " + std::filesystem::canonical( requested ).string() + "\r\n" );

      oResponse.AppendMessageBody( content + "\r\n" );
   }
   catch( const std::exception& e )
   {
      oResponse.AppendMessageBody( e.what() );
   }

   return oResponse;
}

HttpResponse FileServlet::HandleCreateDirectoryRequest( const std::filesystem::path& requested ) const noexcept
{
   HttpResponse oResponse( HttpVersion10, HttpStatusConflict, "FAILED TO CREATE DIRECTORIES" );
   try
   {
      if( std::filesystem::create_directories( requested ) )
         oResponse = HttpResponse{ HttpVersion10, HttpStatusCreated, "CREATED DIRECTORIES" };
      else
         throw std::runtime_error( "Failed to create directory(ies) for path: " +
                                   std::filesystem::absolute( requested ).string() + "\r\n" );
   }
   catch( const std::exception& e )
   {
      oResponse.AppendMessageBody( e.what() );
   }

   return oResponse;
}

HttpResponse FileServlet::HandleWriteFileRequest( const std::filesystem::path& requested,
                                                  const std::string& content ) const noexcept
{
   auto lasWrite = std::chrono::time_point_cast<std::chrono::seconds>(
         std::filesystem::last_write_time( requested )
      ).time_since_epoch();

   std::time_t t = lasWrite.count();

   std::stringstream timeBuffer;
   timeBuffer << std::ctime( &t );

   std::string timeStamp = timeBuffer.str();
   timeStamp = reduce( timeStamp, "_", ":" );
   timeStamp = reduce( timeStamp, "_", "\n" );
   timeStamp = reduce( timeStamp, "_", " " );

   try
   {
      auto newPath = requested.parent_path() / ( timeStamp + requested.filename().string() );
      std::filesystem::rename( requested, newPath );
   }
   catch( std::exception e )
   {
      HttpResponse oResponse( HttpVersion10, HttpStatusConflict, "FAILED TO BACKUP OLD FILE" );
      oResponse.AppendMessageBody( e.what() );
      return oResponse;
   }

   return HandleCreateFileRequest( requested, content );
}
