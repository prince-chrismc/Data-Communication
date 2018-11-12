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

void AppController::Initialize()
{
   if( ! m_Socket.Open( "127.0.0.1", 8080 ) )
      throw std::runtime_error( "Failed to open on port" );
}

void AppController::Run()
{
      auto output = TextProtocol::Message{TextProtocol::PacketType::ACK, 
                                          TextProtocol::SequenceNumber{0},
                                          TextProtocol::IpV4Address{0xEEAA0044},
                                          TextProtocol::PortNumber{8080}
      };

      std::cout<< "Client attempting to send the ";
      output.Print();

      m_Socket.Send( output );

      TextProtocol::Message input = m_Socket.Receive();

      std::cout << "Client obtained the following ";
      input.Print();

   m_Socket.Close();

   return;
}