# Data-Communication [![Build Status](https://travis-ci.org/prince-chrismc/Data-Communication.svg?branch=master)](https://travis-ci.org/prince-chrismc/Data-Communication)
This is a C++ networking  repository to contain my work for Data Communications and the Internet (COMP445) from Concordia Universirty during Fall 2018.

### Table of Contents
- [Modules](#Modules)
  - [Libraries Installation](#Libraries-Installation)
  - [Setting Enviroment Variables](#Setting-Enviroment-Variables)
 - [Building](#Building)
  
## Modules
#### [Assignments](https://github.com/prince-chrismc/Data-Communication/tree/master/Assignments)
- [Curl](https://github.com/prince-chrismc/Data-Communication/tree/master/Assignments/Curl)
- [Image Height Map](https://github.com/prince-chrismc/Data-Communication/tree/master/Assignments/File-Server)

#### Libraries
- [Command Line Parser](https://github.com/prince-chrismc/Cli-Parser)
- [Network Sockets](https://github.com/prince-chrismc/Simple-Socket)

## Building
This is a very simple library, it is very easy to build and configure and the 'third-party' dependencies are self contained. Make sure to recursively clone the repository to obtain a copy of the submodules. To build and install, use CMake to generate the files required for your platform and execute the appropriate build command or procedure. No cmake options/settings should be changed or configured.

Unix Systems: The command is make which produce a release and debug version depending on the CMAKE_BUILD_TYPE specified.
Windows Systems: The usual MSVC files can be build through the IDE or via command line interface.

There is no installation of any kind.
