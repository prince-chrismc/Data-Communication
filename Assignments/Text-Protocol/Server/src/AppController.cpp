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

#include "AppController.h"
#include "Message.h"
#include <iostream>
#include "Socket.h"
#include <thread>

using namespace std::chrono_literals;

void AppController::Initialize()
{
   if( !m_Socket.Initialize() )
      throw std::runtime_error( "Failed to init socket" );

   if( ! m_Socket.Listen( "127.0.0.1", 8080 ) )
      throw std::runtime_error( "Failed to listen on port" );
}

void AppController::Run()
{
   while( true )
   {
      //m_Socket.Select();

      auto input = TextProtocol::Socket::Receive( m_Socket );

      if( input.has_value() )
      {
         std::cout << "Server >> obtained the following ";
         input->Print();

         TextProtocol::Socket::Send( m_Socket, *input );
      }

      std::this_thread::sleep_for( 100ms );
   }

   m_Socket.Close();

   return;
}