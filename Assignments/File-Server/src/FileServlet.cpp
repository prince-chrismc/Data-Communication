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

using Http::Version;
using Http::Status;

FileServlet::FileServlet( const std::string& path ) : m_Path( path )
{
   if( !std::filesystem::is_directory( m_Path ) ) throw std::logic_error( "File exploration must happen from a directory!" );
}

HttpResponse FileServlet::HandleRequest( const HttpRequest& request ) const noexcept
{
   switch( request.GetMethod() )
   {
   case Http::RequestMethod::Get:
      return HandleGetRequest( request );
   case Http::RequestMethod::Post:
      return HandlePostRequest( request );
   default:
      break;
   }

   return{ Http::Version::v10, Status::MethodNotAllowed, "SERVER ONLY ACCEPTS GET AND POST METHODS" };
}

HttpResponse FileServlet::HandleGetRequest( const HttpRequest& request ) const noexcept
{
   if( request.GetUri().find( "/.." ) != std::string::npos )
      return{ Http::Version::v10, Status::Forbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

   const std::filesystem::path oRequested = m_Path / request.GetUri().substr( 1 );

   if( !std::filesystem::exists( oRequested ) )
      return{ Http::Version::v10, Status::NotFound, "NOT FOUND" };

   if( std::filesystem::is_directory( oRequested ) )
      return HandleDirectoryRequest( oRequested );

   if( std::filesystem::is_regular_file( oRequested ) )
      return HandleFileRequest( oRequested );

   return{ Http::Version::v10, Status::NotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
}

HttpResponse FileServlet::HandleDirectoryRequest( const std::filesystem::path& requested ) const noexcept
{
   HttpResponse oResponse( Http::Version::v10, Status::Ok, "OK" );

   oResponse.SetContentType( Http::ContentType::Text );

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
   HttpResponse oResponse( Http::Version::v10, Status::Ok, "OK" );
   oResponse.SetContentType( FileExtensionToContentType( requested ) );
   oResponse.SetMessageHeader( "Content-Disposition", "inline" );

   if( oResponse.GetContentType() != Http::ContentType::Png )
      oResponse.AppendMessageBody( "File: " + std::filesystem::canonical( requested ).string() + "\r\n" );

   std::ifstream fileReader( requested.string(), std::ios::in | std::ios::binary | std::ios::ate );
   if( !fileReader ) return{ Http::Version::v10, Status::InternalServerError, "COULD NOT LOAD FILE" };

   const size_t size = fileReader.tellg();
   std::string fileBuffer( size, '\0' ); // construct buffer
   fileReader.seekg( 0 ); // rewind
   fileReader.read( fileBuffer.data(), size );

   oResponse.AppendMessageBody( fileBuffer );

   return oResponse;
}

Http::ContentType FileServlet::FileExtensionToContentType( const std::filesystem::path& requested ) const noexcept
{
   if( !std::filesystem::is_regular_file( requested ) )
      return Http::ContentType::Invalid;

   if( !requested.has_extension() )
      return Http::ContentType::Text;

   if( requested.filename().string().find( ".vcxproj" ) != std::string::npos )
      return Http::ContentType::Xml;

   if( requested.extension() == ".html" )
      return Http::ContentType::Html;

   if( requested.extension() == ".json" )
      return Http::ContentType::Json;

   if( requested.extension() == ".yml" )
      return Http::ContentType::Yaml;

   if( requested.extension() == ".png" )
      return Http::ContentType::Png;

   return Http::ContentType::Text;
}


HttpResponse FileServlet::HandlePostRequest( const HttpRequest& request ) const noexcept
{
   if( request.GetUri().find( "/.." ) != std::string::npos )
      return{ Http::Version::v10, Status::Forbidden, "NICE TRY ACCESSING FORBIDDEN DIRECTORY OF FILE SYSTEM" };

   const std::filesystem::path oRequested = std::filesystem::absolute( m_Path / request.GetUri().substr( 1 ) );

   if( !std::filesystem::exists( oRequested ) )
      return HandleCreateItemRequest( oRequested, request.GetBody() );

   if( std::filesystem::is_directory( oRequested ) )
      return{ Http::Version::v10, Status::BadRequest, "CAN NOT CREATE EXISTING DIRECTORIES" };

   if( std::filesystem::is_regular_file( oRequested ) )
      return HandleWriteFileRequest( oRequested, request.GetBody() );

   return{ Http::Version::v10, Status::NotImplemented, "ONLY SUPPORTS DIRS AND FILES" };
}

HttpResponse FileServlet::HandleCreateItemRequest( const std::filesystem::path& requested,
                                                   const std::string& content ) const noexcept
{
   if( requested.has_extension() || content.length() )
      return HandleCreateFileRequest( requested, content );
   else
      return HandleCreateDirectoryRequest( requested );
}

void CreateParentDirectories( const std::filesystem::path& requested )
{
   const auto parentPath = requested.parent_path();
   if( !std::filesystem::exists( parentPath ) )
      if( !std::filesystem::create_directories( parentPath ) )
         throw std::runtime_error( "Failed to create directory(ies) for path: " +
                                   std::filesystem::canonical( parentPath ).string() + "\r\n" );
}

HttpResponse FileServlet::HandleCreateFileRequest( const std::filesystem::path& requested,
                                                   const std::string& content ) const noexcept
{
   HttpResponse oResponse( Http::Version::v10, Status::Conflict, "FAILED TO CREATE FILE" );
   try
   {
      CreateParentDirectories( requested );

      std::ofstream fileWriter( requested.string() );

      if( !fileWriter )
         throw std::runtime_error( "Failed to create file located at: " +
                                   std::filesystem::canonical( requested ).string() + "\r\n" );

      fileWriter << content << std::endl;
      fileWriter.close();

      oResponse = HttpResponse( Http::Version::v10, Status::Created, "CREATED FILE" );
      oResponse.SetContentType( FileExtensionToContentType( requested ) );
      oResponse.SetMessageHeader( "Content-Disposition", "inline" );

      if( oResponse.GetContentType() != Http::ContentType::Png )
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
   HttpResponse oResponse( Http::Version::v10, Status::Conflict, "FAILED TO CREATE DIRECTORIES" );
   try
   {
      if( std::filesystem::create_directories( requested ) )
         oResponse = HttpResponse{ Http::Version::v10, Status::Created, "CREATED DIRECTORIES" };
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
   timeStamp = reduce( timeStamp, "_" );

   try
   {
      auto newPath = requested.parent_path() / ( timeStamp + requested.filename().string() );
      std::filesystem::rename( requested, newPath );
   }
   catch( const std::exception& e )
   {
      HttpResponse oResponse( Http::Version::v10, Status::Conflict, "FAILED TO BACKUP OLD FILE" );
      oResponse.AppendMessageBody( e.what() );
      return oResponse;
   }

   return HandleCreateFileRequest( requested, content );
}
