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

using namespace std::chrono_literals;

int main( int argc, char** argv )
{
   try
   {
      // oApp.Initialize();

      // oApp.Run();

      HttpServer oServer;

      oServer.RegisterServlet( "/", nullptr );
      oServer.RegisterServlet( "/helloWorld", nullptr );
      oServer.RegisterServlet( "/test", nullptr );
      oServer.RegisterServlet( "/test/123", nullptr );

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
