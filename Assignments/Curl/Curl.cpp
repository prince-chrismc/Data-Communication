// Curl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CmdLineParser.h"

int main(int argc, char** argv)
{
   std::cout << "Hello World!\n";

   CommandLineParser oParser( argc, argv );

   return 0;
}
